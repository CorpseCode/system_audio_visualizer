#define _USE_MATH_DEFINES
#include <cmath>
#include "fft_processor.h"

#include <complex>
#include <algorithm>

using Complex = std::complex<double>;

static bool isPowerOfTwo(int x) { return x > 0 && (x & (x - 1)) == 0; }

FFTProcessor::FFTProcessor(int window_size, int output_bins)
    : windowSize_(window_size),
      outBinsCount_(output_bins),
      smoothingAlpha_(0.6),
      ringBuffer_(window_size * 2, 0.0f),
      ringPos_(0)
{
    if (!isPowerOfTwo(windowSize_))
    {
        windowSize_ = 2048;
        ringBuffer_.assign(windowSize_ * 2, 0.0f);
    }
}

void FFTProcessor::SetSmoothing(double alpha)
{
    smoothingAlpha_ = std::clamp(alpha, 0.0, 0.999);
}

void FFTProcessor::PushSamples(const float *samples, int sampleCount)
{
    int bufSize = static_cast<int>(ringBuffer_.size());
    for (int i = 0; i < sampleCount; i++)
    {
        ringBuffer_[ringPos_] = samples[i];
        ringPos_ = (ringPos_ + 1) % bufSize;
    }
}

bool FFTProcessor::GetBins(std::vector<double> &outBins)
{
    int bufSize = static_cast<int>(ringBuffer_.size());
    if (bufSize < windowSize_)
        return false;

    std::vector<float> window(windowSize_);

    int start = (ringPos_ - windowSize_ + bufSize) % bufSize;
    for (int i = 0; i < windowSize_; ++i)
    {
        int idx = (start + i) % bufSize;
        window[i] = ringBuffer_[idx];
    }

    applyHannWindow(window);
    computeFFT(window, outBins);
    return true;
}

void FFTProcessor::applyHannWindow(std::vector<float> &data)
{
    int N = static_cast<int>(data.size());
    for (int n = 0; n < N; ++n)
    {
        double w = 0.5 * (1.0 - cos(2.0 * M_PI * n / (N - 1)));
        data[n] *= static_cast<float>(w);
    }
}

static void bitReverseSwap(std::vector<Complex> &a)
{
    int n = static_cast<int>(a.size());
    int j = 0;
    for (int i = 0; i < n; ++i)
    {
        if (i < j)
            std::swap(a[i], a[j]);
        int m = n >> 1;
        while (j >= m && m > 0)
        {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
}

void FFTProcessor::computeFFT(const std::vector<float> &window, std::vector<double> &magOut)
{
    int N = windowSize_;
    std::vector<Complex> data(N);
    for (int i = 0; i < N; ++i)
    {
        data[i] = Complex(window[i], 0.0);
    }

    bitReverseSwap(data);

    for (int len = 2; len <= N; len <<= 1)
    {
        double angle = -2.0 * M_PI / len;
        Complex wlen(cos(angle), sin(angle));
        for (int i = 0; i < N; i += len)
        {
            Complex w(1.0, 0.0);
            int half = len >> 1;
            for (int j = 0; j < half; ++j)
            {
                Complex u = data[i + j];
                Complex v = data[i + j + half] * w;
                data[i + j] = u + v;
                data[i + j + half] = u - v;
                w *= wlen;
            }
        }
    }

    int half = N / 2;
    std::vector<double> mags(half);
    double maxMag = 1e-12;

    for (int i = 0; i < half; ++i)
    {
        mags[i] = std::abs(data[i]);
        if (mags[i] > maxMag)
            maxMag = mags[i];
    }

    magOut.assign(outBinsCount_, 0.0);

    for (int b = 0; b < outBinsCount_; ++b)
    {
        double low = pow((double)half, (double)b / outBinsCount_);
        double high = pow((double)half, (double)(b + 1) / outBinsCount_);

        int ilo = std::max(0, static_cast<int>(floor(low)));
        int ihi = std::min(half - 1, static_cast<int>(ceil(high)));

        double sum = 0.0;
        int count = std::max(1, ihi - ilo + 1);

        for (int k = ilo; k <= ihi; ++k)
            sum += mags[k];

        double avg = sum / count;
        double val = avg / (maxMag + 1e-12);

        double scaled = log10(1.0 + 9.0 * val);
        magOut[b] = std::clamp(scaled, 0.0, 1.0);
    }
}

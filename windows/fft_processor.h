#ifndef FFT_PROCESSOR_H_
#define FFT_PROCESSOR_H_

#include <vector>

// Simple FFT processor (radix-2 iterative). No external deps.
class FFTProcessor
{
public:
    // window_size must be power of two (e.g., 1024, 2048, 4096)
    explicit FFTProcessor(int window_size = 2048, int output_bins = 64);

    // push interleaved stereo floats (L,R,L,R,...). This mixes to mono internally.
    void PushSamples(const float *samples, int sampleCount);

    // returns true if a window is ready (and grabs magnitudes into outBins)
    bool GetBins(std::vector<double> &outBins);

    // set smoothing factor 0..1 (0=no smoothing, 0.8 heavy)
    void SetSmoothing(double alpha);

private:
    int windowSize_;
    int outBinsCount_;
    double smoothingAlpha_;

    std::vector<float> ringBuffer_;
    int ringPos_;

    // internal
    void computeFFT(const std::vector<float> &window, std::vector<double> &magOut);
    void applyHannWindow(std::vector<float> &data);
};

#endif // FFT_PROCESSOR_H_

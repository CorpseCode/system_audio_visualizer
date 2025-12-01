#include "wasapi_capture.h"

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <mmreg.h>
#include <avrt.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

#pragma comment(lib, "Avrt.lib")

#define LOG(x) std::cout << "[WASAPI] " << x << std::endl;

static bool TryCoInitialize(DWORD flags, bool &needsUninit);

// ---------------------------------------------------------
// Device change notification client
// ---------------------------------------------------------
class DeviceNotificationClient : public IMMNotificationClient
{
public:
    explicit DeviceNotificationClient(std::function<void()> cb)
        : ref_(1), cb_(std::move(cb)) {}

    ULONG STDMETHODCALLTYPE AddRef() override { return InterlockedIncrement(&ref_); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG v = InterlockedDecrement(&ref_);
        if (v == 0)
            delete this;
        return v;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **out) override
    {
        if (!out)
            return E_POINTER;
        if (riid == __uuidof(IUnknown) || riid == __uuidof(IMMNotificationClient))
        {
            *out = this;
            AddRef();
            return S_OK;
        }
        *out = nullptr;
        return E_NOINTERFACE;
    }

    // IMMNotificationClient
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR) override
    {
        if (flow == eRender && role == eConsole)
        {
            LOG("Default device changed.");
            if (cb_)
                cb_();
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(LPCWSTR, DWORD) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR) override { return S_OK; }
    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(LPCWSTR, const PROPERTYKEY) override { return S_OK; }

private:
    LONG ref_;
    std::function<void()> cb_;
};

// ---------------------------------------------------------
// WasapiCapture::Impl — keep private
// ---------------------------------------------------------
struct WasapiCapture::Impl
{
    Impl()
        : device(nullptr),
          audio(nullptr),
          capture(nullptr),
          format(nullptr),
          enumerator(nullptr),
          notifier(nullptr),
          running(false),
          sampleRate(48000) {}

    ~Impl()
    {
        running = false;
        if (thread.joinable())
            thread.join();

        if (enumerator && notifier)
            enumerator->UnregisterEndpointNotificationCallback(notifier);

        if (notifier)
            notifier->Release();
        if (enumerator)
            enumerator->Release();
        if (format)
            CoTaskMemFree(format);
        if (capture)
            capture->Release();
        if (audio)
            audio->Release();
        if (device)
            device->Release();
    }

    IMMDevice *device;
    IAudioClient *audio;
    IAudioCaptureClient *capture;
    WAVEFORMATEX *format;

    IMMDeviceEnumerator *enumerator;
    DeviceNotificationClient *notifier;

    std::atomic<bool> running;
    std::thread thread;
    int sampleRate;

    std::function<void(const float *, int)> callback;
    std::mutex cbLock;
};

// ---------------------------------------------------------
// Constructor
// ---------------------------------------------------------
WasapiCapture::WasapiCapture() : impl_(new Impl) {}

WasapiCapture::~WasapiCapture()
{
    Stop();
}

// ---------------------------------------------------------
// Internal helper (now *inside* class, so no access errors)
// ---------------------------------------------------------
void WasapiCapture::HandleDeviceChange()
{
    LOG("Handling device change...");

    bool wasRunning = impl_->running;

    Stop();

    // Release previous objects
    if (impl_->capture)
    {
        impl_->capture->Release();
        impl_->capture = nullptr;
    }
    if (impl_->audio)
    {
        impl_->audio->Release();
        impl_->audio = nullptr;
    }
    if (impl_->device)
    {
        impl_->device->Release();
        impl_->device = nullptr;
    }
    if (impl_->format)
    {
        CoTaskMemFree(impl_->format);
        impl_->format = nullptr;
    }

    LOG("Reinitializing after device change...");
    if (Initialize())
    {
        if (wasRunning)
        {
            std::function<void(const float *, int)> cb;
            {
                std::lock_guard<std::mutex> lock(impl_->cbLock);
                cb = impl_->callback;
            }
            Start(cb);
        }
    }
}

// ---------------------------------------------------------
// Initialize
// ---------------------------------------------------------
bool WasapiCapture::Initialize()
{
    LOG("Initialize()");

    bool needUninit = false;
    TryCoInitialize(COINIT_MULTITHREADED, needUninit);

    HRESULT hr;

    // Create enumerator if needed
    if (!impl_->enumerator)
    {
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                              __uuidof(IMMDeviceEnumerator),
                              reinterpret_cast<void **>(&impl_->enumerator));
        if (FAILED(hr))
            return false;

        impl_->notifier = new DeviceNotificationClient([this]()
                                                       { HandleDeviceChange(); });
        impl_->enumerator->RegisterEndpointNotificationCallback(impl_->notifier);
    }

    // Get default device
    hr = impl_->enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &impl_->device);
    if (FAILED(hr))
        return false;

    // Activate client
    hr = impl_->device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                                 reinterpret_cast<void **>(&impl_->audio));
    if (FAILED(hr))
        return false;

    // Mix format
    hr = impl_->audio->GetMixFormat(&impl_->format);
    if (FAILED(hr))
        return false;

    impl_->sampleRate = impl_->format->nSamplesPerSec;

    // Loopback initialization
    hr = impl_->audio->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0, 0,
        impl_->format,
        nullptr);

    if (FAILED(hr))
        return false;

    // Capture service
    hr = impl_->audio->GetService(__uuidof(IAudioCaptureClient),
                                  reinterpret_cast<void **>(&impl_->capture));
    if (FAILED(hr))
        return false;

    return true;
}

// ---------------------------------------------------------
// Start capture
// ---------------------------------------------------------
bool WasapiCapture::Start(std::function<void(const float *, int)> cb)
{
    if (!impl_->audio || !impl_->capture)
        return false;

    {
        std::lock_guard<std::mutex> lock(impl_->cbLock);
        impl_->callback = cb;
    }

    if (impl_->running)
        return true;

    impl_->running = true;
    impl_->thread = std::thread([this]()
                                {
        bool needUninit = false;
        TryCoInitialize(COINIT_MULTITHREADED, needUninit);

        impl_->audio->Start();

        WAVEFORMATEX* wf = impl_->format;
        int ch = wf->nChannels;

        while (impl_->running) {
            UINT32 packet = 0;
            impl_->capture->GetNextPacketSize(&packet);
            if (!packet) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            BYTE* data;
            UINT32 frames;
            DWORD flags;
            impl_->capture->GetBuffer(&data, &frames, &flags, nullptr, nullptr);

            size_t total = (size_t)frames * ch;
            std::vector<float> out(total);

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                std::fill(out.begin(), out.end(), 0.f);
            } else {
                float* f = (float*)data;
                for (size_t i = 0; i < total; i++) out[i] = f[i];
            }

            std::function<void(const float*, int)> cb;
            {
                std::lock_guard<std::mutex> lock(impl_->cbLock);
                cb = impl_->callback;
            }

            cb(out.data(), (int)out.size());

            impl_->capture->ReleaseBuffer(frames);
        }

        impl_->audio->Stop();
        if (needUninit) CoUninitialize(); });

    return true;
}

// ---------------------------------------------------------
// Stop
// ---------------------------------------------------------
void WasapiCapture::Stop()
{
    impl_->running = false;
    if (impl_->thread.joinable())
        impl_->thread.join();
}

// ---------------------------------------------------------
// Sample rate
// ---------------------------------------------------------
int WasapiCapture::sample_rate() const
{
    return impl_->sampleRate;
}

// ---------------------------------------------------------
// TryCoInitialize
// ---------------------------------------------------------
static bool TryCoInitialize(DWORD flags, bool &needsUninit)
{
    HRESULT hr = CoInitializeEx(nullptr, flags);
    if (hr == RPC_E_CHANGED_MODE)
    {
        needsUninit = false;
        return true;
    }
    needsUninit = SUCCEEDED(hr);
    return SUCCEEDED(hr);
}

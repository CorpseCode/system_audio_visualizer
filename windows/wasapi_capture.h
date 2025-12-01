#ifndef WASAPI_CAPTURE_H_
#define WASAPI_CAPTURE_H_

#include <functional>
#include <memory>

class WasapiCapture
{
public:
    WasapiCapture();
    ~WasapiCapture();

    bool Initialize();
    bool Start(std::function<void(const float *samples, int sampleCount)> callback);
    void Stop();

    int sample_rate() const;

    // Called internally when default audio device changes
    void HandleDeviceChange();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

#endif

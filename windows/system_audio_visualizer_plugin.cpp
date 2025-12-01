#include "system_audio_visualizer_plugin.h"
#include "wasapi_capture.h"
#include "fft_processor.h"

#include <flutter/encodable_value.h>
#include <flutter/event_channel.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <mutex>
#include <vector>
#include <chrono>

namespace system_audio_visualizer
{
  using namespace flutter;

  class SystemAudioVisualizerPluginImpl : public Plugin
  {
  public:
    explicit SystemAudioVisualizerPluginImpl(BinaryMessenger *messenger)
        : messenger_(messenger),
          capture_(std::make_unique<WasapiCapture>()),
          fft_(2048, 64)
    {

      // ------------------ Method Channel ------------------
      method_channel_ = std::make_unique<MethodChannel<EncodableValue>>(
          messenger_, "system_audio_visualizer/methods",
          &StandardMethodCodec::GetInstance());

      method_channel_->SetMethodCallHandler(
          [this](const MethodCall<EncodableValue> &call,
                 std::unique_ptr<MethodResult<EncodableValue>> result)
          {
            if (call.method_name() == "start")
            {
              bool ok = StartCapture();
              if (ok)
                result->Success();
              else
                result->Error("init_failed", "Failed to initialize WASAPI capture");
            }
            else if (call.method_name() == "stop")
            {
              StopCapture();
              result->Success();
            }
            else
            {
              result->NotImplemented();
            }
          });

      // ------------------ Event Channel ------------------
      event_channel_ = std::make_unique<EventChannel<EncodableValue>>(
          messenger_, "system_audio_visualizer/fft",
          &StandardMethodCodec::GetInstance());

      auto handler =
          std::make_unique<StreamHandlerFunctions<EncodableValue>>(
              [this](
                  const EncodableValue *,
                  std::unique_ptr<EventSink<EncodableValue>> &&events)
              {
                std::lock_guard<std::mutex> lock(event_mutex_);
                event_sink_ = std::move(events);
                return nullptr;
              },
              [this](const EncodableValue *)
              {
                std::lock_guard<std::mutex> lock(event_mutex_);
                event_sink_.reset();
                return nullptr;
              });

      event_channel_->SetStreamHandler(std::move(handler));
    }

    ~SystemAudioVisualizerPluginImpl() override { StopCapture(); }

  private:
    // ----------------------- Audio Capture -----------------------
    bool StartCapture()
    {
      if (running_)
        return true;

      if (!capture_->Initialize())
      {
        return false;
      }

      bool started = capture_->Start(
          [this](const float *samples, int sampleCount)
          {
            // Convert stereo -> mono if needed
            std::vector<float> mono;
            mono.reserve(sampleCount);

            if (sampleCount >= 2 && (sampleCount % 2) == 0)
            {
              for (int i = 0; i + 1 < sampleCount; i += 2)
              {
                float m = 0.5f * (samples[i] + samples[i + 1]);
                mono.push_back(m);
              }
            }
            else
            {
              mono.assign(samples, samples + sampleCount);
            }

            // Feed FFT
            fft_.PushSamples(mono.data(), (int)mono.size());

            // Try output
            std::vector<double> bins;
            if (fft_.GetBins(bins))
            {
              SendBins(bins);
            }
          });

      if (!started)
        return false;

      running_ = true;
      return true;
    }

    void StopCapture()
    {
      if (!running_)
        return;
      capture_->Stop();
      running_ = false;
    }

    // ----------------------- Streaming to Dart -----------------------
    void SendBins(const std::vector<double> &bins)
    {
      std::lock_guard<std::mutex> lock(event_mutex_);
      if (!event_sink_)
        return;

      EncodableList list;
      list.reserve(bins.size());
      for (double v : bins)
      {
        list.emplace_back(EncodableValue(v));
      }

      event_sink_->Success(EncodableValue(list));
    }

    // Members
    BinaryMessenger *messenger_;
    std::unique_ptr<MethodChannel<EncodableValue>> method_channel_;
    std::unique_ptr<EventChannel<EncodableValue>> event_channel_;
    std::unique_ptr<EventSink<EncodableValue>> event_sink_;
    std::mutex event_mutex_;

    std::unique_ptr<WasapiCapture> capture_;
    FFTProcessor fft_;
    std::atomic<bool> running_{false};
  };

  // ----------------------------- Registration -----------------------------
  void SystemAudioVisualizerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarWindows *registrar)
  {
    auto plugin =
        std::make_unique<SystemAudioVisualizerPluginImpl>(registrar->messenger());
    registrar->AddPlugin(std::move(plugin));
  }

} // namespace system_audio_visualizer

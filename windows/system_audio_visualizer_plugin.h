#ifndef FLUTTER_PLUGIN_SYSTEM_AUDIO_VISUALIZER_PLUGIN_H_
#define FLUTTER_PLUGIN_SYSTEM_AUDIO_VISUALIZER_PLUGIN_H_

#include <flutter/plugin_registrar_windows.h>

namespace system_audio_visualizer
{

    class SystemAudioVisualizerPlugin
    {
    public:
        static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);
    };

} // namespace system_audio_visualizer

#endif // FLUTTER_PLUGIN_SYSTEM_AUDIO_VISUALIZER_PLUGIN_H_

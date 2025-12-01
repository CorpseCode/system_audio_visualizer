#include "include/system_audio_visualizer/system_audio_visualizer_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "system_audio_visualizer_plugin.h"

void SystemAudioVisualizerPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  system_audio_visualizer::SystemAudioVisualizerPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}

import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'system_audio_visualizer_platform_interface.dart';

/// An implementation of [SystemAudioVisualizerPlatform] that uses method channels.
class MethodChannelSystemAudioVisualizer extends SystemAudioVisualizerPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('system_audio_visualizer');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}

import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'system_audio_visualizer_method_channel.dart';

abstract class SystemAudioVisualizerPlatform extends PlatformInterface {
  /// Constructs a SystemAudioVisualizerPlatform.
  SystemAudioVisualizerPlatform() : super(token: _token);

  static final Object _token = Object();

  static SystemAudioVisualizerPlatform _instance = MethodChannelSystemAudioVisualizer();

  /// The default instance of [SystemAudioVisualizerPlatform] to use.
  ///
  /// Defaults to [MethodChannelSystemAudioVisualizer].
  static SystemAudioVisualizerPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [SystemAudioVisualizerPlatform] when
  /// they register themselves.
  static set instance(SystemAudioVisualizerPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}

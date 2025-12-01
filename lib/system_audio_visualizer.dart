import 'dart:async';
import 'package:flutter/services.dart';

class SystemAudioVisualizer {
  static const MethodChannel _method = MethodChannel(
    'system_audio_visualizer/methods',
  );

  static const EventChannel _fftChannel = EventChannel(
    'system_audio_visualizer/fft',
  );

  /// Start capture with optional FFT config.
  static Future<void> start({int fftSize = 2048, int bins = 64}) {
    return _method.invokeMethod('start', {'fftSize': fftSize, 'bins': bins});
  }

  static Future<void> stop() => _method.invokeMethod('stop');

  /// FFT bin stream
  static Stream<List<double>> get fftStream =>
      _fftChannel.receiveBroadcastStream().map((dynamic event) {
        final list = event as List<dynamic>;
        return list.map((e) => (e as num).toDouble()).toList();
      });
}

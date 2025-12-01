import 'package:flutter/material.dart';

/// Global, customizable configuration for all visualizers.
class VisualizerConfig {
  /// Color gradient used by most visualizers
  final List<Color> colors;

  /// Width of bars/lines/etc.
  final double thickness;

  /// Glow radius, applied via MaskFilter
  final double glow;

  /// Corner radius (bars, shapes)
  final double radius;

  /// Vertical scaling factor for intensity
  final double intensity;

  /// FFT smoothing factor (0 = raw, 0.9 = smooth)
  final double smoothing;

  /// Peak hold decay speed
  final double peakDecay;

  /// Frequency mapping: "linear", "log", "exp"
  final String frequencyMap;

  const VisualizerConfig({
    this.colors = const [
      Color(0xFF00EFFF),
      Color(0xFF0077FF),
      Color(0xFF8A2BE2),
    ],
    this.thickness = 10.0,
    this.glow = 2.0,
    this.radius = 12.0,
    this.intensity = 0.95,
    this.smoothing = 0.80,
    this.peakDecay = 0.25,
    this.frequencyMap = "log",
  });

  VisualizerConfig copyWith({
    List<Color>? colors,
    double? thickness,
    double? glow,
    double? radius,
    double? intensity,
    double? smoothing,
    double? peakDecay,
    String? frequencyMap,
  }) {
    return VisualizerConfig(
      colors: colors ?? this.colors,
      thickness: thickness ?? this.thickness,
      glow: glow ?? this.glow,
      radius: radius ?? this.radius,
      intensity: intensity ?? this.intensity,
      smoothing: smoothing ?? this.smoothing,
      peakDecay: peakDecay ?? this.peakDecay,
      frequencyMap: frequencyMap ?? this.frequencyMap,
    );
  }
}

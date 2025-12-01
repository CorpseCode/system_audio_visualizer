import 'package:flutter/material.dart';
import 'visualizer_config.dart';

class VisualizerTheme {
  static VisualizerConfig neon() => const VisualizerConfig(
        colors: [Colors.cyan, Colors.blue, Colors.purple],
        glow: 25,
        thickness: 6,
      );

  static VisualizerConfig fire() => const VisualizerConfig(
        colors: [Colors.orange, Colors.redAccent, Colors.deepPurple],
        glow: 32,
        intensity: 1.6,
        radius: 2,
      );

  static VisualizerConfig anime() => const VisualizerConfig(
        colors: [Color(0xFF00F0FF), Color(0xFF3B82F6), Color(0xFF8B5CF6)],
        smoothing: 0.75,
        glow: 30,
        intensity: 1.5,
        thickness: 4,
        radius: 12,
      );
}

import 'package:flutter/material.dart';
import '../core/visualizer_base.dart';
import '../core/visualizer_config.dart';

class NeonBarsVisualizer extends AudioVisualizerWidget {
  const NeonBarsVisualizer({
    super.key,
    required super.bins,
    required super.config,
  });

  @override
  Widget build(_) {
    return CustomPaint(
      painter: _HologramBarsPainter(bins, config),
      size: Size.infinite,
    );
  }
}

class _HologramBarsPainter extends CustomPainter {
  final List<double> bins;
  final VisualizerConfig config;

  _HologramBarsPainter(this.bins, this.config);

  @override
  void paint(Canvas canvas, Size size) {
    final pad = 20.0;

    // Reduce bar count to 12
    final barCount = 12;
    final groupSize = (bins.length / barCount).floor();

    // compress bins into 12 values
    final List<double> bars = [];
    for (int i = 0; i < barCount; i++) {
      double sum = 0;
      for (int j = 0; j < groupSize; j++) {
        sum += bins[i * groupSize + j];
      }
      bars.add(sum / groupSize);
    }

    final barWidth = (size.width - pad * 2) / barCount;
    final barRadius = Radius.circular(config.radius);

    for (int i = 0; i < barCount; i++) {
      final x = pad + i * barWidth;
      final h = bars[i] * size.height * config.intensity;

      final rect = RRect.fromLTRBR(
        x + barWidth * 0.15,
        size.height - h - pad,
        x + barWidth * 0.85,
        size.height - pad,
        barRadius,
      );

      // hologram gradient border
      final borderPaint = Paint()
        ..style = PaintingStyle.stroke
        ..strokeWidth = 3
        ..shader = LinearGradient(
          colors: config.colors,
          begin: Alignment.topCenter,
          end: Alignment.bottomCenter,
        ).createShader(rect.outerRect);

      // translucent fill
      final fillPaint = Paint()
        ..color = Colors.white.withValues(alpha: 0.07)
        ..style = PaintingStyle.fill;

      canvas.drawRRect(rect, fillPaint);
      canvas.drawRRect(rect, borderPaint);
    }
  }

  @override
  bool shouldRepaint(_) => true;
}

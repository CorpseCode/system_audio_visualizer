import 'dart:math' as math;
import 'package:flutter/material.dart';

import '../core/visualizer_config.dart';

class AnimeWaveVisualizer extends StatefulWidget {
  const AnimeWaveVisualizer({
    super.key,
    required this.bins,
    required this.config,
  });

  final List<double> bins;
  final VisualizerConfig config;

  @override
  State<AnimeWaveVisualizer> createState() => _AnimeWaveVisualizerState();
}

class _AnimeWaveVisualizerState extends State<AnimeWaveVisualizer>
    with SingleTickerProviderStateMixin {
  late final AnimationController _controller;

  @override
  void initState() {
    super.initState();

    _controller = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 4),
    )..repeat();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(context) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (_, __) {
        return CustomPaint(
          painter: _TripleWavePainter(
            widget.bins,
            widget.config,
            _controller.value, // flowing gradient phase
          ),
          size: Size.infinite,
        );
      },
    );
  }
}

class _TripleWavePainter extends CustomPainter {
  final List<double> bins;
  final VisualizerConfig config;
  final double flow;

  _TripleWavePainter(this.bins, this.config, this.flow);

  // Safe average even if bins are small
  double _avg(int start, int end) {
    if (bins.isEmpty) return 0.0;
    double sum = 0;
    final safeEnd = math.min(end, bins.length);
    for (int i = start; i < safeEnd; i++) {
      sum += bins[i];
    }
    return sum / (safeEnd - start).clamp(1, 999999);
  }

  @override
  void paint(Canvas canvas, Size size) {
    final double pad = 20.0;
    final double bandHeight = (size.height - pad * 2) / 3;

    final double yTop = pad + bandHeight * 0.5;
    final double yMid = pad + bandHeight * 1.5;
    final double yLow = pad + bandHeight * 2.5;

    final double width = size.width - pad * 2;

    // FREQUENCY GROUPING
    final double voice = _avg(10, 35); // mids
    final double bass = _avg(0, 10); // low frequencies
    final double trebleRaw = _avg(35, 64); // high frequencies

    // Boost treble so it becomes visible
    final double treble = math.pow(trebleRaw, 0.5) * 2.3;

    // VISUAL AMPLITUDES
    final double Avoice = voice * size.height * 0.15 * config.intensity;
    final double Abass = bass * size.height * 0.25 * config.intensity;
    final double Atreble = treble * size.height * 0.12 * config.intensity;

    // Bass center pulse effect
    final double bassPulse = bass * 22.0;

    const int steps = 260;
    const double freq = 8.0; // more waves across the width

    // Instagram-style gradient palette
    final igColors = [
      const Color(0xFFFF512F), // orange-red
      const Color(0xFFDD2476), // magenta
      const Color(0xFFFF6FD8), // pink
      const Color(0xFFF83600), // neon red
      const Color(0xFFFE8C00), // orange-yellow
    ];

    // Gradient movement (strong flow)
    final double offset = (flow * 4) % 4;

    Shader flowingShader() {
      return LinearGradient(
        colors: igColors,
        begin: Alignment(-1 + offset, 0),
        end: Alignment(1 + offset, 0),
        tileMode: TileMode.mirror,
      ).createShader(Rect.fromLTWH(0, 0, size.width, size.height));
    }

    // DRAW A SINGLE WAVE
    void drawWave(double baseY, double amp, double pulse) {
      final path = Path()..moveTo(pad, baseY);

      for (int i = 0; i <= steps; i++) {
        final t = i / steps;
        final x = pad + width * t;

        // Center pulse only for bass wave
        final pulseMod = (1 - (t - 0.5).abs() * 2).clamp(0.0, 1.0) * pulse;

        final y = baseY + math.sin(t * math.pi * freq) * (amp + pulseMod);

        path.lineTo(x, y);
      }

      final paint = Paint()
        ..strokeWidth = config.thickness * 0.45
        ..style = PaintingStyle.stroke
        ..strokeCap = StrokeCap.round
        ..shader = flowingShader();

      canvas.drawPath(path, paint);
    }

    // Render order: bottom → top
    drawWave(yLow, Atreble, 0); // Treble
    drawWave(yTop, Avoice, 0); // Voice
    drawWave(yMid, Abass, bassPulse); // Bass with pulse
  }

  @override
  bool shouldRepaint(_) => true;
}

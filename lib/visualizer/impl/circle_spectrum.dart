import 'dart:math';
import 'package:flutter/material.dart';
import '../core/visualizer_config.dart';

class CircleSpectrumVisualizer extends StatefulWidget {
  const CircleSpectrumVisualizer({
    super.key,
    required this.bins,
    required this.config,
  });

  final List<double> bins;
  final VisualizerConfig config;

  @override
  State<CircleSpectrumVisualizer> createState() =>
      _CircleSpectrumVisualizerState();
}

class _CircleSpectrumVisualizerState extends State<CircleSpectrumVisualizer>
    with SingleTickerProviderStateMixin {
  late final AnimationController _controller;

  @override
  void initState() {
    super.initState();
    _controller = AnimationController(
      vsync: this,
      duration: const Duration(seconds: 6),
    )..repeat();
  }

  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(_) {
    return AnimatedBuilder(
      animation: _controller,
      builder: (_, __) {
        return CustomPaint(
          painter: _PetalRingPainter(
            widget.bins,
            widget.config,
            _controller.value * 2 * pi, // 0 → 2π rotation
          ),
          size: Size.infinite,
        );
      },
    );
  }
}

class _PetalRingPainter extends CustomPainter {
  final List<double> bins;
  final VisualizerConfig config;
  final double rotation; // flowing radial gradient rotation

  _PetalRingPainter(this.bins, this.config, this.rotation);

  @override
  void paint(Canvas canvas, Size size) {
    final center = size.center(Offset.zero);
    final radius = min(size.width, size.height) * 0.27;

    final int count = 51;
    final stride = bins.length ~/ count;
    final List<double> reduced = [];

    for (int i = 0; i < count; i++) {
      double sum = 0;
      for (int j = 0; j < stride; j++) {
        sum += bins[i * stride + j];
      }
      reduced.add(sum / stride);
    }

    final step = 2 * pi / count;

    // IG-style neon rotating gradient palette
    final List<Color> igColors = [
      const Color(0xFFFF512F),
      const Color(0xFFDD2476),
      const Color(0xFFFF6FD8),
      const Color(0xFFF83600),
      const Color(0xFFFE8C00),
    ];

    for (int i = 0; i < count; i++) {
      double prev = reduced[(i - 1) % count];
      double next = reduced[(i + 1) % count];
      double smoothed = reduced[i] * 0.6 + prev * 0.2 + next * 0.2;

      final angle = i * step;

      // Slightly stronger shaping
      double amp = pow(smoothed, 1.8) * radius * config.intensity;

      final start = Offset(
        center.dx + cos(angle) * radius,
        center.dy + sin(angle) * radius,
      );

      final mid = Offset(
        center.dx + cos(angle) * (radius + amp * 0.6),
        center.dy + sin(angle) * (radius + amp * 0.6),
      );

      final end = Offset(
        center.dx + cos(angle) * (radius + amp),
        center.dy + sin(angle) * (radius + amp),
      );

      final path = Path()
        ..moveTo(start.dx, start.dy)
        ..quadraticBezierTo(mid.dx, mid.dy, end.dx, end.dy);

      // Dynamic radial flowing sweep gradient
      final paint = Paint()
        ..strokeWidth = config.thickness * 0.9
        ..style = PaintingStyle.stroke
        ..strokeCap = StrokeCap.round
        ..shader = SweepGradient(
          colors: igColors,
          startAngle: rotation, // this rotates the gradient!
          endAngle: rotation + 2 * pi, // full spin
          tileMode: TileMode.repeated,
        ).createShader(Rect.fromCircle(center: center, radius: radius * 1.2));

      canvas.drawPath(path, paint);
    }

    // Soft inner ring shimmer
    final innerPaint = Paint()
      ..strokeWidth = config.thickness * 0.55
      ..style = PaintingStyle.stroke
      ..color = Colors.white.withOpacity(0.09);

    canvas.drawCircle(center, radius * 0.92, innerPaint);
  }

  @override
  bool shouldRepaint(_) => true;
}

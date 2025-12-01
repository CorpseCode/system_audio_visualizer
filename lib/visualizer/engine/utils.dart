import 'package:flutter/material.dart';

Paint glowPaint(List<Color> colors, MaskFilter glow) {
  final paint = Paint()
    ..shader = LinearGradient(colors: colors).createShader(
      const Rect.fromLTWH(0, 0, 1, 1),
    )
    ..maskFilter = glow;
  return paint;
}

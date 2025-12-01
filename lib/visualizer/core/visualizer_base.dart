import 'package:flutter/material.dart';
import 'visualizer_config.dart';

abstract class AudioVisualizerWidget extends StatelessWidget {
  final List<double> bins;
  final VisualizerConfig config;

  const AudioVisualizerWidget({
    super.key,
    required this.bins,
    required this.config,
  });
}

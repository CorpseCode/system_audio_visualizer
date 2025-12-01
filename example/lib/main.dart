import 'package:flutter/material.dart';
import 'package:system_audio_visualizer/system_audio_visualizer.dart';
import 'package:system_audio_visualizer/visualizer/engine/smoothing.dart';
import 'package:system_audio_visualizer/visualizer/core/visualizer_config.dart';
import 'package:system_audio_visualizer/visualizer/core/visualizer_type.dart';
import 'package:system_audio_visualizer/visualizer/impl/neon_bars.dart';
import 'package:system_audio_visualizer/visualizer/impl/anime_wave.dart';
import 'package:system_audio_visualizer/visualizer/impl/circle_spectrum.dart';

void main() => runApp(const AVExample());

class AVExample extends StatefulWidget {
  const AVExample({super.key});
  @override
  State<AVExample> createState() => _AVExampleState();
}

class _AVExampleState extends State<AVExample> {
  final config = VisualizerConfig();
  late SmoothFilter smoother;
  List<double> bins = List.filled(64, 0);

  VisualizerType type = VisualizerType.neonBars;

  @override
  void initState() {
    super.initState();
    smoother = SmoothFilter(64, config.smoothing);
    SystemAudioVisualizer.start();

    SystemAudioVisualizer.fftStream.listen((raw) {
      setState(() => bins = smoother.apply(raw));
    });
  }

  Widget buildViz() {
    switch (type) {
      case VisualizerType.neonBars:
        return NeonBarsVisualizer(bins: bins, config: config);
      case VisualizerType.animeWave:
        return AnimeWaveVisualizer(bins: bins, config: config);
      case VisualizerType.circleSpectrum:
        return CircleSpectrumVisualizer(bins: bins, config: config);

    }
  }

  @override
  Widget build(context) {
    return MaterialApp(
      home: Scaffold(
        backgroundColor: Colors.black,
        body: Stack(
          children: [
            Positioned.fill(child: buildViz()),
            Positioned(
              top: 40,
              right: 20,
              child: DropdownButton<VisualizerType>(
                dropdownColor: Colors.black,
                value: type,
                items: VisualizerType.values.map((v) {
                  return DropdownMenuItem(
                    value: v,
                    child: Text(
                      v.name,
                      style: const TextStyle(color: Colors.white),
                    ),
                  );
                }).toList(),
                onChanged: (v) => setState(() => type = v!),
              ),
            ),
          ],
        ),
      ),
    );
  }
}

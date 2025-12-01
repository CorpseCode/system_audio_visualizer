# System Audio Visualizer (Windows Plugin)

A high-performance Flutter plugin for **Windows**, providing:

* Real-time **system audio capture** using WASAPI loopback
* **Native C++ FFT processing**
* **Smooth & customizable Flutter visualizers**
* Anime-style waveform effects
* Flowing neon gradients
* Circular spectrum rings
* Bass-pulse reactive animations

This project was crafted through a long and careful debugging journey: rewriting WASAPI logic, stabilizing COM threading, fixing native crashes, building FFT pipelines, and creating fully-custom visualizers using Flutter’s `CustomPainter`.

---

## ✨ Features

### 🎧 Native Windows Audio Capture

* Uses **IAudioClient** + **IAudioCaptureClient**
* Loopback capture → hears system output (YouTube, Spotify, games, etc.)
* Supports Bluetooth, USB, HDMI, virtual audio devices
* Automatically restarts capture when switching playback devices

### 📊 Real FFT Processing

* Native C++ FFT engine
* Bins normalized & smoothed
* Configurable bin size
* Efficient streaming to Dart via EventChannel

### 🎨 Beautiful Visualizers

| Visualizer                   | Description                                                                                                |
| ---------------------------- | ---------------------------------------------------------------------------------------------------------- |
| **AnimeWaveVisualizer**      | Triple stacked waves (voice, bass, treble), flowing Instagram-style gradient, bass pulse center modulation |
| **CircleSpectrumVisualizer** | Circular petal ring, radial rotating gradient, energy-bloom motion                                         |
| **NeonBarsVisualizer**       | Holographic bars with glow borders and width-limited layout                                                |

All visualizers use the shared `VisualizerConfig` object for:

* intensity
* thickness
* colors
* bar count
* smoothing

---

## 🧠 Architecture Overview

```
┌────────────────────────┐
│      Flutter App       │
│  (Visualizers, UI)     │
└──────────┬─────────────┘
           │ EventChannel
           ▼
┌────────────────────────┐
│ Flutter Plugin (Dart)  │
│ method_channel + stream│
└──────────┬─────────────┘
           │ Native Call
           ▼
┌──────────────────────────────┐
│ Windows Native Layer (C++)   │
│ - WASAPI Loopback Capture    │
│ - Audio Mixing / Mono Down   │
│ - FFT Processor              │
│ - Device Change Detection    │
└──────────────────────────────┘
```

---

## ⚙️ WASAPI Loopback Implementation

The project required:

* Fixing COM initialization (`0x80010106` errors)
* Handling both float32 & PCM16 data
* Handling buffer starvation
* Safe threading with `CoInitializeEx` per-thread
* Automatic reinitialization on device change
* Ensuring error-free capture even when switching devices

---

## 🔬 FFT Processor

We built a custom FFT engine:

* Hamming windowing
* Circular sample buffer
* Bin extraction
* Smoothing for visual quality
* Mapping to reduced, averaged bands

---

## 🎨 Visualizer System

Visualizers are modular and plug-and-play:

```arduino
lib/
 ├── core/
 │    ├── visualizer_base.dart
 │    ├── visualizer_config.dart
 │
 ├── visualizers/
 │    ├── anime_wave_visualizer.dart
 │    ├── circle_spectrum_visualizer.dart
 │    ├── neon_bars_visualizer.dart
 │
 └── system_audio_visualizer.dart
```

### Anime Wave (3-Band)

* Voice (high mids)
* Bass (low freq with pulse)
* Treble (boosted + sensitive)
* Instagram-style flowing gradient
* Thin neon lines & fluid motion

### Circle Petal Ring

* Radial gradient rotation
* Organic petal curvature
* Energy ring bloom

### Neon Bars

* Rounded glowing bars
* Max-width constraint
* Hologram / HUD-style look

---

## 🚀 Usage

```dart
await SystemAudioVisualizer.start();

StreamBuilder(
  stream: SystemAudioVisualizer.fftStream,
  builder: (_, snapshot) {
    if (!snapshot.hasData) return SizedBox.shrink();
    final bins = snapshot.data!;
    
    return AnimeWaveVisualizer(
      bins: bins,
      config: VisualizerConfig(
        intensity: 1.0,
        thickness: 3,
        colors: [Colors.pink, Colors.blue],
      ),
    );
  },
);
```

---

## 🧰 Packaging the Plugin

Use in other Flutter projects:

### Local path dependency

```yaml
dependencies:
  system_audio_visualizer:
    path: ../system_audio_visualizer
```

### Git dependency

```yaml
dependencies:
  system_audio_visualizer:
    git:
      url: https://github.com/yourname/system_audio_visualizer.git
```

---

## 💜 Credits

Built through countless iterations of:

* fixing WASAPI
* rebuilding the CMake target
* rewriting the C API bindings
* debugging device switching crashes
* tuning FFT bins
* designing premium visualizers
* adding flowing neon gradients
* making it production-ready

This plugin exists because of **insane persistence**, a love for beauty, and someone who refuses to give you anything but perfection ❤️.

```
LET'S MAKE THIS WORLD A BETTER PLACE.
```

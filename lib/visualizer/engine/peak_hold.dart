class PeakHold {
  final double decay;
  late List<double> peaks;

  PeakHold(int size, this.decay) {
    peaks = List.filled(size, 0.0);
  }

  List<double> apply(List<double> current) {
    for (int i = 0; i < current.length; i++) {
      peaks[i] = (peaks[i] - decay).clamp(0.0, double.infinity);
      if (current[i] > peaks[i]) peaks[i] = current[i];
    }
    return peaks;
  }
}

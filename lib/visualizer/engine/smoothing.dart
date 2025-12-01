class SmoothFilter {
  final double factor;
  late List<double> value;

  SmoothFilter(int size, this.factor) {
    value = List.filled(size, 0.0);
  }

  List<double> apply(List<double> input) {
    for (int i = 0; i < input.length; i++) {
      value[i] = value[i] * factor + input[i] * (1 - factor);
    }
    return value;
  }
}

import 'dart:math';

class FrequencyMapping {
  static double map(String mode, int i, int n) {
    switch (mode) {
      case "linear":
        return i / n;
      case "exp":
        return pow(i / n, 2).toDouble();
      default: // log
        return log(1 + i.toDouble()) / log(n.toDouble());
    }
  }
}

typedef double float_t;
typedef float float_input_t;

struct Data {
  int *N;
  float_t *sum;
  float_t *sumsq;
};

struct ExportSATData {
  int N;
  double sum;
  double sumsq;
};

void compute_sat(uniform int width,
                 uniform int height,
                 const uniform float_input_t src[],
                 const uniform int mask[],
                 uniform Data table) {
  uniform const float_t BIG = (1 << 24);
  // idx is the global "current index"
  for (uniform int y = 0, idx = 0; y < height; ++y) {
    uniform int N = 0;
    uniform float_t sum = 0.0f;
    uniform float_t sumsq = 0.0f;
    for (uniform int x = 0; x < width; ++x, ++idx) {
      if (mask[idx] && src[idx] < BIG) {
        N += 1;
        sum += src[idx];
        sumsq += src[idx] * src[idx];
      }
      if (y == 0) {
        table.N[idx] = N;
        table.sum[idx] = sum;
        table.sumsq[idx] = sumsq;
      } else {
        table.N[idx] = N + table.N[idx - width];
        table.sum[idx] = sum + table.sum[idx - width];
        table.sumsq[idx] = sumsq + table.sumsq[idx - width];
      }
    }
  }
}

export void dispersion_threshold(uniform const float_input_t *uniform src,
                                 uniform const int *uniform mask,
                                 uniform const float gain,
                                 uniform int *uniform dst,
                                 uniform int width,
                                 uniform int height,
                                 uniform int kernel_xsize,
                                 uniform int kernel_ysize,
                                 uniform float sigma_b,
                                 uniform float sigma_s,
                                 uniform float threshold,
                                 uniform int min_count,
                                 uniform ExportSATData *uniform satData) {
  uniform Data table;
  // Dedicated AOS layout
  table.N = uniform new int[width * height];
  table.sum = uniform new float_t[width * height];
  table.sumsq = uniform new float_t[width * height];

  compute_sat(width, height, src, mask, table);

  // Copy over the SAT for diagnostics
  if (satData) {
    for (uniform int i = 0; i < (width * height); ++i) {
      satData[i].N = table.N[i];
      satData[i].sum = table.sum[i];
      satData[i].sumsq = table.sumsq[i];
    }
  }

  foreach (y = 0 ... height, x = 0 ... width) {
    // Get the left, right, top, bottom of the kernel window
    int left = x - kernel_xsize - 1;
    int right = min(width - 1, x + kernel_xsize);
    int top = y - kernel_ysize - 1;
    int bottom = min(height - 1, y + kernel_ysize);

    int idx_br = bottom * width + right;
    int idx_tl = top * width + left;
    int idx_tr = top * width + right;
    int idx_bl = bottom * width + left;

    int N = table.N[idx_br];
    float sum = table.sum[idx_br];
    double sumsq = table.sumsq[idx_br];

    if (left >= 0 && top >= 0) {
      // Slightly unnatural and wrong(ish) way to do this - can blow out
      // the top of the float range. It's the same way as the original
      // dispersion though, which makes comparisons easier in float.
      // area = data_br - (data_bl + data_tr) + data_tl;
      N = N - (table.N[idx_bl] + table.N[idx_tr]) + table.N[idx_tl];
      sum = sum - (table.sum[idx_bl] + table.sum[idx_tr]) + table.sum[idx_tl];
      sumsq = sumsq - (table.sumsq[idx_bl] + table.sumsq[idx_tr]) + table.sumsq[idx_tl];
      // area = data_br - data_bl - data_tr + data_tl;
    } else if (left >= 0) {
      // area = data_br - data_bl;
      N = N - table.N[idx_bl];
      sum = sum - table.sum[idx_bl];
      sumsq = sumsq - table.sumsq[idx_bl];
    } else if (top >= 0) {
      N = N - table.N[idx_tr];
      sum = sum - table.sum[idx_tr];
      sumsq = sumsq - table.sumsq[idx_tr];
    }

    int idx = y * width + x;

    if (mask[idx] && N >= min_count && sum >= 0 && src[idx] > threshold) {
      double a = N * sumsq - sum * sum;
      double b = N * src[idx] - sum;
      double c = gain * sum * (N - 1 + sigma_b * sqrt((float)(2 * (N - 1))));
      double d = sigma_s * sqrt(gain * sum * N);
      // bool above_back = a > c;
      // bool above_strong = b > d;
#if DEBUG

      if (x == 2828 && y == 1625) {
        print("Debug DISPERSION x = %, y = %\n", x, y);
        print("top: %\n", top);
        print("left: %\n", left);
        print("bottom: %\n", bottom);
        print("right: %\n", right);
        print("area N:  %\n     sum:   %\n     sumsq:  %\n", N, sum, sumsq);
        print("tl N:  %\n     sum:   %\n     sumsq:  %\n",
              table.N[idx_tl],
              table.sum[idx_tl],
              table.sumsq[idx_tl]);
        print("tr N:  %\n     sum:   %\n     sumsq:  %\n",
              table.N[idx_tr],
              table.sum[idx_tr],
              table.sumsq[idx_tr]);
        print("bl N:  %\n     sum:   %\n     sumsq:  %\n",
              table.N[idx_bl],
              table.sum[idx_bl],
              table.sumsq[idx_bl]);
        print("br N:  %\n     sum:   %\n     sumsq:  %\n",
              table.N[idx_br],
              table.sum[idx_br],
              table.sumsq[idx_br]);
        print("Result:\n a > c   %5.4f > %5.4f\n b > c   %5.4f > %5.4f\n", a, c, b, d);
      }
#endif
      dst[idx] = a > c && b > d;
    } else {
      dst[idx] = false;
    }
  }

  delete[] table.N;
  delete[] table.sum;
  delete[] table.sumsq;
}
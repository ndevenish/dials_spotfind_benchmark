typedef float float_t;
typedef double float_input_t;

struct Data {
  int *N;
  float *sum;
  double *sumsq;
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
    uniform float sum = 0.0f;
    uniform double sumsq = 0.0f;
    for (uniform int x = 0; x < width; ++x, ++idx) {
      if (mask[idx] && src[idx] < BIG) {
        N += 1;
        sum += src[idx];
        sumsq += (src[idx] * src[idx]);  // - average;
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

uniform float average(const uniform float_input_t *uniform src,
                      uniform const int *uniform mask,
                      uniform int width,
                      uniform int height) {
  uniform const float_t BIG = (1 << 24);
  // get the average
  double sum = 0;
  int N = 0;
  foreach (idx = 0 ...(width * height)) {
    if (mask[idx] && src[idx] < BIG) {
      sum += src[idx] * src[idx];
      N += 1;
    }
  }
  return reduce_add(sum) / reduce_add(N);
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
  table.sumsq = uniform new double[width * height];

  // Possible improvement: subtract average src**2 from SAT sumsq table,
  // so that full float range is used (and non-monotonically)
  // uniform float average = average(src, mask, width, height);
  // print("Average: %\n", average);

  compute_sat(width, height, src, mask, table);

  // Copy over the SAT for diagnostics
  if (satData) {
    for (uniform int i = 0; i < (width * height); ++i) {
      satData[i].N = table.N[i];
      satData[i].sum = table.sum[i];
      satData[i].sumsq = table.sumsq[i];
    }
  }

  foreach (y = kernel_ysize + 1 ... height - kernel_ysize,
           x = kernel_xsize... width - kernel_xsize) {
    // Get the left, right, top, bottom of the kernel window
    int left = x - kernel_xsize - 1;
    int right = x + kernel_xsize;
    int top = y - kernel_ysize - 1;
    int bottom = y + kernel_ysize;

    int idx_br = bottom * width + right;
    int idx_tl = top * width + left;
    int idx_tr = top * width + right;
    int idx_bl = bottom * width + left;

    int N = table.N[idx_br];
    float sum = table.sum[idx_br];
    double sumsq = table.sumsq[idx_br];

    N = N - table.N[idx_bl] - table.N[idx_tr] + table.N[idx_tl];
    sum = sum - table.sum[idx_bl] - table.sum[idx_tr] + table.sum[idx_tl];
    sumsq = sumsq - table.sumsq[idx_bl] - table.sumsq[idx_tr] + table.sumsq[idx_tl];

    int idx = y * width + x;

    if (mask[idx] && N >= min_count && sum >= 0 && src[idx] > threshold) {
      double a = N * sumsq - sum * sum;
      double b = N * src[idx] - sum;
      double c = gain * sum * (N - 1 + sigma_b * sqrt((float)(2 * (N - 1))));
      double d = sigma_s * sqrt(gain * sum * N);

      dst[idx] = a > c && b > d;
    } else {
      dst[idx] = false;
    }
  }

  delete[] table.N;
  delete[] table.sum;
  delete[] table.sumsq;
}
typedef float float_t;

// Convenience struct for storing arrays
struct Data {
  int *N;
  float *sum;
  float *sumsq;
};

void compute_sat(uniform int width,
                 uniform int height,
                 const uniform float_t src[],
                 const uniform int mask[],
                 uniform Data table) {
  uniform const float_t BIG = (1 << 24);
  // idx is the global "current index"
  for (uniform int y = 0, idx = 0; y < height; ++y) {
    uniform int N = 0;
    uniform float_t sum = 0.0f;
    uniform float_t sumsq = 0.0f;
    for (uniform int x = 0; x < height; ++x, ++idx) {
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

export void dispersion_threshold(uniform const float_t *uniform src,
                                 uniform const int *uniform mask,
                                 uniform const float_t *uniform gain,
                                 uniform int *uniform dst,
                                 uniform int width,
                                 uniform int height,
                                 uniform int kernel_xsize,
                                 uniform int kernel_ysize,
                                 uniform float sigma_s,
                                 uniform float sigma_b,
                                 uniform float threshold,
                                 uniform int min_count) {
  uniform Data table;

  // Dedicated AOS layout
  table.N = uniform new int[width * height];
  table.sum = uniform new float_t[width * height];
  table.sumsq = uniform new float_t[width * height];

  compute_sat(width, height, src, mask, table);

  // Performance hack: Ignore a kernel_sized strip on edges
  // This can be special cased later, but means that the compiler recognises
  // the linear blocks of memory access and can avoid a gather instruction
  // - on the edges, some lanes have incoherent lookups
  foreach (y = (kernel_ysize + 1)... height - kernel_ysize,
           x = (kernel_xsize + 1)... width - kernel_xsize) {
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
    int sum = table.sum[idx_br];
    int sumsq = table.sumsq[idx_br];

    // Slightly unnatural and wrong(ish) way to do this - can blow out
    // the top of the float range. It's the same way as the original
    // dispersion though, which makes comparisons easier in float.
    // area = data_br - (data_bl + data_tr) + data_tl;
    N = N - (table.N[idx_bl] + table.N[idx_tr]) + table.N[idx_tl];
    sum = sum - (table.sum[idx_bl] + table.sum[idx_tr]) + table.sum[idx_tl];
    sumsq = sumsq - (table.sumsq[idx_bl] + table.sumsq[idx_tr]) + table.sumsq[idx_tl];

    int idx = y * width + x;

    if (mask[idx] && N >= min_count && sum >= 0 && src[idx] > threshold) {
      double a = N * sumsq - sum * sum;
      double b = N * src[idx] - sum;
      double c = gain[idx] * sum * (N - 1 + sigma_b * sqrt((float)(2 * (N - 1))));
      double d = sigma_s * sqrt(gain[idx] * sum * N);
      // bool above_back = a > c;
      // bool above_strong = b > d;
      dst[idx] = a > c && b > d;
    } else {
      dst[idx] = false;
    }
  }

  delete[] table.N;
  delete[] table.sum;
  delete[] table.sumsq;
}
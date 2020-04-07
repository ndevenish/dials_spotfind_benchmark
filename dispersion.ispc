// #define float_t float
typedef float float_t;

struct Data {
  int N;
  float sum;
  float sumsq;
};

struct Data operator+(struct Data a, struct Data b) {
  struct Data result;
  result.N = a.N + b.N;
  result.sum = a.sum + b.sum;
  result.sumsq = a.sumsq + b.sumsq;
  return result;
}
struct Data operator-(struct Data a, struct Data b) {
  struct Data result;
  result.N = a.N - b.N;
  result.sum = a.sum - b.sum;
  result.sumsq = a.sumsq - b.sumsq;
  return result;
}

// soa<8> Data

void compute_sat(uniform int width,
                 uniform int height,
                 const uniform float_t src[],
                 const uniform int mask[],
                 uniform Data table[]) {
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
        table[idx].N = N;
        table[idx].sum = sum;
        table[idx].sumsq = sumsq;
      } else {
        table[idx].N = N + table[idx - width].N;
        table[idx].sum = sum + table[idx - width].sum;
        table[idx].sumsq = sumsq + table[idx - width].sumsq;
      }
    }
  }
}

// export void compute_threshold(uniform const float_t *uniform src,
//                                  uniform const bool *uniform mask,
//                                  uniform int width,
//                                  uniform int height,
//                                  uniform int kernel_xsize,
//                                  uniform int kernel_ysize,
//                                  uniform float sigma_s,
//                                  uniform float sigma_b,
//                                  uniform float threshold,
//                                  uniform int min_count)
// {

// }

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
  uniform Data *uniform table = uniform new Data[width * height];

  compute_sat(width, height, src, mask, table);

  foreach (y = 0 ... height, x = 0 ... width) {
    // Get the left, right, top, bottom of the kernel window
    int left = x - kernel_xsize - 1;
    int right = min(width - 1, x + kernel_xsize);
    int top = y - kernel_ysize - 1;
    int bottom = min(height - 1, y + kernel_ysize);

    Data data_br = table[bottom * width + right];
    Data area;
    if (left >= 0 && top >= 0) {
      Data data_tl = table[top * width + left];
      Data data_tr = table[top * width + right];
      Data data_bl = table[bottom * width + left];

      // Slightly unnatural and wrong(ish) way to do this - can blow out
      // the top of the float range. It's the same way as the original
      // dispersion though, which makes comparisons easier in float.
      area = data_br - (data_bl + data_tr) + data_tl;
      // area = data_br - data_bl - data_tr + data_tl;
    } else if (left >= 0) {
      Data data_bl = table[bottom * width + left];
      area = data_br - data_bl;
    } else if (top >= 0) {
      Data data_tr = table[top * width + right];
      area = data_br - data_tr;
    } else {
      area = data_br;
    }

    int idx = y * width + x;

    if (mask[idx] && area.N >= min_count && area.sum >= 0 && src[idx] > threshold) {
      double a = area.N * area.sumsq - area.sum * area.sum;
      double b = area.N * src[idx] - area.sum;
      double c =
        gain[idx] * area.sum * (area.N - 1 + sigma_b * sqrt((float)(2 * (area.N - 1))));
      double d = sigma_s * sqrt(gain[idx] * area.sum * area.N);
      // bool above_back = a > c;
      // bool above_strong = b > d;
      dst[idx] = a > c && b > d;
    } else {
      dst[idx] = false;
    }
  }

  delete[] table;
}
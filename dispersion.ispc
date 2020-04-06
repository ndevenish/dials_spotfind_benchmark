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
                 const uniform bool mask[],
                 uniform Data table[]) {
  uniform const float_t BIG = (1 << 24);
  // idx is the global "current index"
  for (uniform int x = 0, idx = 0; x < width; ++x) {
    uniform int N = 0;
    uniform float_t sum = 0.0f;
    uniform float_t sumsq = 0.0f;
    for (uniform int y = 0; y < height; ++y, ++idx) {
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

export void dispersion_threshold(uniform const float_t src[],
                                 uniform const bool mask[],
                                 uniform const float_t gain[],
                                 uniform bool dst[],
                                 uniform int width,
                                 uniform int height,
                                 uniform int kernel_xsize,
                                 uniform int kernel_ysize,
                                 uniform float sigma_s,
                                 uniform float sigma_b,
                                 uniform float threshold,
                                 uniform int min_count) {
  uniform Data* uniform table = uniform new Data[width * height];

  print("Computing SAT");

  compute_sat(width, height, src, mask, table);
  print("Calculating Thresholding");

  foreach (y = 0 ... height, x = 0 ... width) {
    // Get the left, right, top, bottom of the kernel window
    int left = max(0, x - kernel_xsize - 1);
    int right = min(width - 1, x + kernel_xsize);
    int top = max(0, y - kernel_ysize - 1);
    int bottom = min(height - 1, y + kernel_ysize);
    Data data_tl = table[top * width + left];
    Data data_tr = table[top * width + right];
    Data data_bl = table[bottom * width + left];
    Data data_br = table[bottom * width + right];
    Data area = data_br - data_bl - data_tr + data_tl;

    int idx = y * width + x;
    // do the thresholding calculations
    if (mask[idx] && area.N >= min_count && area.sum >= 0 && src[idx] > threshold) {
      double a = area.N * area.sumsq - area.sum * area.sum;
      double b = area.N * src[idx] - area.sum;
      double c =
        gain[idx] * area.sum * (area.N - 1 + sigma_b * sqrt((float)(2 * (area.N - 1))));
      double d = sigma_s * sqrt(gain[idx] * area.sum * area.N);
      dst[idx] = a > c && b > d;
    } else {
      dst[idx] = false;
    }
  }

  delete[] table;
}
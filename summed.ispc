typedef double float_t;
typedef float float_input_t;

export void dispersion_summed(uniform const float_input_t *uniform src,
                              uniform const int *uniform mask,
                              uniform const float gain,
                              uniform int *uniform dst,
                              uniform int width,
                              uniform int height,
                              uniform int kernel_xsize_,
                              uniform int kernel_ysize_,
                              uniform float sigma_b,
                              uniform float sigma_s,
                              uniform float threshold,
                              uniform int min_count) {
  uniform const int kernel_xsize = 3;
  uniform const int kernel_ysize = 3;
  foreach (y = kernel_ysize... height - kernel_ysize,
           x = kernel_xsize... width - kernel_xsize) {
    int N = 0;
    float sum = 0.0f;
    float sumsq = 0.0;
    for (int py = y - kernel_ysize; py < y + kernel_ysize; ++py) {
      for (int px = x - kernel_xsize; px < x + kernel_xsize; ++px) {
        int pidx = py * width + px;
        N += src[pidx];
        sum += src[pidx];
        sumsq += src[pidx] * src[pidx];
      }
    }
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
}
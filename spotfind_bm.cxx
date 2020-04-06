#include <benchmark/benchmark.h>

#include "spotfind.h"
#include "dials/algorithms/image/threshold/local.h"

using dials::algorithms::DispersionExtendedThreshold;
using dials::algorithms::DispersionThreshold;


static void BM_standard_dispersion(benchmark::State& state) {
  ImageSource<double> src;
  for (auto _ : state) {
    auto algo = DispersionThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold(src.src, src.mask, src.dst);
  }
}
BENCHMARK(BM_standard_dispersion)->Unit(benchmark::kMillisecond);

static void BM_extended_dispersion(benchmark::State& state) {
  ImageSource<double> src;
  for (auto _ : state) {
    auto algo = DispersionExtendedThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold(src.src, src.mask, src.dst);
  }
}
BENCHMARK(BM_extended_dispersion)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

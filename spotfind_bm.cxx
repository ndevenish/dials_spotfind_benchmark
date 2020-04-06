#include <benchmark/benchmark.h>

#include "dials/algorithms/image/threshold/local.h"
#include "itt.h"
#include "spotfind.h"

using dials::algorithms::DispersionExtendedThreshold;
using dials::algorithms::DispersionThreshold;

static void BM_standard_dispersion(benchmark::State& state) {
  ImageSource<double> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion");
  for (auto _ : state) {
    auto algo = DispersionThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold(src.src, src.mask, src.dst);
  }
}
BENCHMARK(BM_standard_dispersion)->Unit(benchmark::kMillisecond);

static void BM_standard_dispersion_gain(benchmark::State& state) {
  ImageSource<double> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion_gain");
  for (auto _ : state) {
    auto algo = DispersionThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold_w_gain(src.src, src.mask, src.gain, src.dst);
  }
}
BENCHMARK(BM_standard_dispersion_gain)->Unit(benchmark::kMillisecond);

static void BM_extended_dispersion(benchmark::State& state) {
  ImageSource<double> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion_extended");
  for (auto _ : state) {
    auto algo = DispersionExtendedThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold(src.src, src.mask, src.dst);
  }
}
BENCHMARK(BM_extended_dispersion)->Unit(benchmark::kMillisecond);

static void BM_extended_dispersion_gain(benchmark::State& state) {
  ImageSource<double> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion_extended_gain");
  for (auto _ : state) {
    auto algo = DispersionExtendedThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold_w_gain(src.src, src.mask, src.gain, src.dst);
  }
}
BENCHMARK(BM_extended_dispersion_gain)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

#include <benchmark/benchmark.h>

#include "dials/algorithms/image/threshold/local.h"
#include "dispersion.h"
#include "itt.h"
#include "spotfind.h"
#include "summed.h"

using dials::algorithms::DispersionExtendedThreshold;
using dials::algorithms::DispersionThreshold;

template <class T>
static void BM_standard_dispersion(benchmark::State& state) {
  ImageSource<T> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion");
  for (auto _ : state) {
    auto algo = DispersionThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold(src.src, src.mask, src.dst);
  }
}
BENCHMARK_TEMPLATE(BM_standard_dispersion, double)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_standard_dispersion, float)->Unit(benchmark::kMillisecond);

template <class T>
static void BM_standard_dispersion_gain(benchmark::State& state) {
  ImageSource<T> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion_gain");
  for (auto _ : state) {
    auto algo = DispersionThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold_w_gain(src.src, src.mask, src.gain, src.dst);
  }
}

BENCHMARK_TEMPLATE(BM_standard_dispersion_gain, double)->Unit(benchmark::kMillisecond);

template <class T>
static void BM_extended_dispersion(benchmark::State& state) {
  ImageSource<T> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion_extended");
  for (auto _ : state) {
    auto algo = DispersionExtendedThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold(src.src, src.mask, src.dst);
  }
}
BENCHMARK_TEMPLATE(BM_extended_dispersion, float)->Unit(benchmark::kMillisecond);
BENCHMARK_TEMPLATE(BM_extended_dispersion, double)->Unit(benchmark::kMillisecond);

template <class T>
static void BM_extended_dispersion_gain(benchmark::State& state) {
  ImageSource<T> src;
  BeginTask task("dials.dispersion.benchmark", "dispersion_extended_gain");
  for (auto _ : state) {
    auto algo = DispersionExtendedThreshold(
      image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
    algo.threshold_w_gain(src.src, src.mask, src.gain, src.dst);
  }
}
BENCHMARK_TEMPLATE(BM_extended_dispersion_gain, double)->Unit(benchmark::kMillisecond);

static void BM_ISPC(benchmark::State& state) {
  ImageSource<double, float> src;

  // Convert mask, dst to int because of https://github.com/ispc/ispc/issues/1709
  std::unique_ptr<int[]> mask(new int[IMAGE_W * IMAGE_H]);
  std::uninitialized_copy(src.mask.begin(), src.mask.end(), &mask[0]);
  std::unique_ptr<int[]> dst(new int[IMAGE_W * IMAGE_H]);
  std::uninitialized_copy(src.dst.begin(), src.dst.end(), &dst[0]);

  BeginTask task("dials.dispersion.benchmark", "initial_ispc");
  for (auto _ : state) {
    ispc::dispersion_threshold(&src.src.front(),
                               mask.get(),
                               1.0,
                               dst.get(),
                               IMAGE_W,
                               IMAGE_H,
                               kernel_size_[0],
                               kernel_size_[1],
                               nsig_b_,
                               nsig_s_,
                               threshold_,
                               min_count_,
                               0);
  }
  task.end();
  // Copy the result back
  for (int i = 0; i < IMAGE_H * IMAGE_W; ++i) {
    src.dst[i] = dst[i];
  }
  // src.write_array("dispersion.tif", src.pre)
  src.write_array("ispc.tif", src.dst);
}
BENCHMARK(BM_ISPC)->Unit(benchmark::kMillisecond);

static void BM_ISPC_summed(benchmark::State& state) {
  ImageSource<float, float> src;

  // Convert mask, dst to int because of https://github.com/ispc/ispc/issues/1709
  std::unique_ptr<int[]> mask(new int[IMAGE_W * IMAGE_H]);
  std::uninitialized_copy(src.mask.begin(), src.mask.end(), &mask[0]);
  std::unique_ptr<int[]> dst(new int[IMAGE_W * IMAGE_H]);
  std::uninitialized_copy(src.dst.begin(), src.dst.end(), &dst[0]);

  BeginTask task("dials.dispersion.benchmark", "ispc_summed");
  for (auto _ : state) {
    ispc::dispersion_summed(&src.src.front(),
                            mask.get(),
                            1.0,
                            dst.get(),
                            IMAGE_W,
                            IMAGE_H,
                            kernel_size_[0],
                            kernel_size_[1],
                            nsig_b_,
                            nsig_s_,
                            threshold_,
                            min_count_);
  }
  task.end();
  // Copy the result back
  // for (int i = 0; i < IMAGE_H * IMAGE_W; ++i) {
  //   src.dst[i] = dst[i];
  // }
  // src.write_array("dispersion.tif", src.pre)
  // src.write_array("ispc.tif", src.dst);
}
BENCHMARK(BM_ISPC_summed)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();

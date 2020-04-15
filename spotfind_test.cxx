#include <iostream>

#include "dials/algorithms/image/threshold/local.h"
#include "dispersion.h"
#include "gtest/gtest.h"
#include "spotfind.h"

using dials::algorithms::DispersionExtendedThreshold;
using dials::algorithms::DispersionThreshold;
using std::cout;
using std::endl;

TEST(Generation, TestInit) {
  ImageSource<double> src;
  src.write_array("test_image.tif", src.src);
}

TEST(Existing, Dispersion) {
  auto algo = DispersionThreshold(
    image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
  ImageSource<double> src;
  algo.threshold(src.src, src.mask, src.dst);

  src.write_array("threshold.tif", src.dst);
  ASSERT_TRUE(src.validate_dst(src.dst));
}

TEST(Existing, DispersionGain) {
  auto algo = DispersionThreshold(
    image_size_, kernel_size_, nsig_b_, nsig_s_, threshold_, min_count_);
  ImageSource<double> src;
  algo.threshold_w_gain(src.src, src.mask, src.gain, src.dst);
  ASSERT_TRUE(src.validate_dst(src.dst));
}

TEST(ISPC, Initial) {
  ImageSource<double, float> src;

  // Convert mask, dst to int because of https://github.com/ispc/ispc/issues/1709
  std::unique_ptr<int[]> mask(new int[IMAGE_W * IMAGE_H]);
  std::uninitialized_copy(src.mask.begin(), src.mask.end(), &mask[0]);
  std::unique_ptr<int[]> dst(new int[IMAGE_W * IMAGE_H]);
  std::fill(&dst[0], dst.get() + (IMAGE_W * IMAGE_H), 0);

  std::vector<ispc::ExportSATData> SAT;
  SAT.resize(IMAGE_W * IMAGE_H);

  ispc::dispersion_threshold(&src.src.front(),
                             mask.get(),
                             1.0,
                             dst.get(),
                             IMAGE_W,
                             IMAGE_H,
                             kernel_size_[0],
                             kernel_size_[1],
                             nsig_s_,
                             nsig_b_,
                             threshold_,
                             min_count_,
                             &SAT.front());
  // Copy the result back
  for (int i = 0; i < IMAGE_H * IMAGE_W; ++i) {
    src.dst[i] = dst[i];
  }

  int mismatch = 0;
  for (int y = 0; y < IMAGE_H; ++y) {
    for (int x = 0; x < IMAGE_W; ++x) {
      const int idx = y * IMAGE_W + x;
      if (SAT[idx].N != src.prefound_SAT[idx].N
          || SAT[idx].sum != src.prefound_SAT[idx].sum
          || SAT[idx].sumsq != src.prefound_SAT[idx].sumsq) {
        printf("Mismatch on %d, %d\n ISPC  %d %f %f\n pref  %d %f %f\n",
               x,
               y,
               SAT[idx].N,
               SAT[idx].sum,
               SAT[idx].sumsq,
               src.prefound_SAT[idx].N,
               src.prefound_SAT[idx].sum,
               src.prefound_SAT[idx].sumsq);
        goto skip;
      }
    }
  }
skip:
  // src.write_array("dispersion.tif", src.pre)
  src.write_array("ispc.tif", src.dst);

  ASSERT_TRUE(src.validate_dst(src.dst));
}

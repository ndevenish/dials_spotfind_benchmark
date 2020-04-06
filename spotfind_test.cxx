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
  ImageSource<float, float> src;
  ispc::dispersion_threshold(&src.src.front(),
                             &src.mask.front(),
                             &src.gain.front(),
                             &src.dst.front(),
                             IMAGE_W,
                             IMAGE_H,
                             kernel_size_[0],
                             kernel_size_[1],
                             nsig_s_,
                             nsig_b_,
                             threshold_,
                             min_count_);
  src.write_array("ispc.tiff", src.dst);
  ASSERT_TRUE(src.validate_dst(src.dst));
  // uniform const float_t src[],
  //   uniform const bool mask[],
  //   uniform const float_t gain[],
  //   uniform bool dst[],
  //   uniform int width, uniform int height,
  //   uniform int kernel_xsize, uniform int kernel_ysize,
  //   uniform float sigma_s, uniform float sigma_b,
  //   uniform float threshold,
  //   uniform int min_count)
  //
}
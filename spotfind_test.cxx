#include <iostream>

#include "dials/algorithms/image/threshold/local.h"
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
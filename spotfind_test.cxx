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

  int count = 0;
  // Do some validation on this
  // Just put some high pixels for now
  src.write_array("threshold.tif", src.dst);
  for (int y = 0; y < IMAGE_W; y += 1) {
    for (int x = 0; x < IMAGE_H; x += 1) {
      if (src.dst(y, x) && (x % 42 != 0) && (y % 42 != 0)) {
        cout << "x " << x << " y " << y << endl;
        count += 1;
      }
      // count += 1;
    }
  }
  cout << "Total: " << count << endl;
}
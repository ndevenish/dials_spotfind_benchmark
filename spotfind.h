#ifndef SPOTFIND_H
#define SPOTFIND_H

#include <algorithm>
#include <iostream>
#include <random>

#include "TinyTIFF/tinytiffwriter.h"

#include <scitbx/array_family/accessors/c_grid.h>
#include <scitbx/array_family/shared.h>
#include "dials/algorithms/image/threshold/local.h"

namespace af = scitbx::af;

const size_t IMAGE_W = 4000;
const size_t IMAGE_H = 4000;

const af::tiny<int, 2> kernel_size_(3, 3);
const af::tiny<int, 2> image_size_(IMAGE_W, IMAGE_H);

const int min_count_ = 2;
const double threshold_ = 0.0;
const double nsig_b_ = 6.0;
const double nsig_s_ = 3.0;

// std::vector<unsigned char> compress_cbf(af::const_ref<T, af::c_grid<2>> image) {
//   // std::vector<unsigned_char
//   auto data_view = image.as_1d();
//   long element_count = data_view.size();

//   std::vector<unsigned char> store;
//   store.reserve(element_count * sizeof(int32_t));
//   unsigned long i_offset = 0, i_pxel = 0;
//   int32_t delta = 0, last_pixel = 0;

//   for (int i_pixel = 0; i_pixel < data_view.size(); ++i_pixel) {
//     delta = data_view[i_pixel] - last_pixel;
//     last_pixel = data_view[i_pixel];
//     if ((-127 <= delta) && (delta <= 127)) {
//       output
//     }
//   }
//       //   std::size_t ysize = src.accessor()[0];
//       // std::size_t xsize = src.accessor()[1];
// }

/// Class to generate sample image data
template <typename T>
class ImageSource {
public:
  ImageSource()
      : destination_store(IMAGE_W * IMAGE_H),
        image_store(IMAGE_W * IMAGE_H),
        mask_store(IMAGE_W * IMAGE_H),
        gain_store(IMAGE_W * IMAGE_H) {
    // Generate the ref objects we will be using
    src = af::const_ref<T, af::c_grid<2>>(image_store.begin(),
                                          af::c_grid<2>(IMAGE_W, IMAGE_H));
    mask = af::const_ref<bool, af::c_grid<2>>(mask_store.begin(),
                                              af::c_grid<2>(IMAGE_W, IMAGE_H));
    gain = af::const_ref<T, af::c_grid<2>>(gain_store.begin(),
                                           af::c_grid<2>(IMAGE_W, IMAGE_H));
    dst = af::ref<bool, af::c_grid<2>>(destination_store.begin(),
                                       af::c_grid<2>(IMAGE_W, IMAGE_H));

    // Don't mask everything
    std::fill(mask_store.begin(), mask_store.end(), true);
    // Gain of 1
    std::fill(gain_store.begin(), gain_store.end(), 1.0);

    // Create the source image
    // src is a const_ref - we want to access it as a ref
    auto writable_src =
      af::ref<T, af::c_grid<2>>(image_store.begin(), af::c_grid<2>(IMAGE_W, IMAGE_H));

    // Create the image
    std::default_random_engine generator(1);
    std::poisson_distribution<int> poisson(1.0);

    // Write a poisson background over the whole image
    // for (int x = 0; x < IMAGE_W; x += 1) {
    //   for (int y = 0; y < IMAGE_W; y += 1) {
    //     writable_src(x, y) = poisson(generator);
    //   }
    // }

    int count = 0;
    // Just put some high pixels for now
    for (int x = 0; x < IMAGE_W; x += 42) {
      for (int y = 0; y < IMAGE_W; y += 42) {
        writable_src(x, y) = 100;
        count += 1;
      }
    }
    std::cout << "Pixels: " << count << std::endl;

    // BOOST_ASSERT(count == 9216);

    // Calculate the SAT - of our sample/random data, but still ensures
    // // that any basic dependencies on SAT-style data are fulfilled
    // auto dp = dials::algorithms::DispersionThreshold(
    //     af::tiny<int, 2>(IMAGE_W, IMAGE_H), kernel_size_, nsig_b_, nsig_s_,
    //     threshold_, min_count_);
    // dp.compute_sat(table, src, mask);

#ifdef BENCHMARK
    benchmark::ClobberMemory();
#endif
  }

  /// Kill the dst explicitly
  void reset_dst() {
    std::fill(destination_store.begin(), destination_store.end(), false);
  }

  template <typename IMSRC>
  void write_array(const char* filename, IMSRC image) {
    // benchmark::ClobberMemory();
    TinyTIFFFile* tif = TinyTIFFWriter_open(filename, 16, IMAGE_W, IMAGE_H);
    std::vector<uint16_t> img;
    img.reserve(image.accessor()[0] * image.accessor()[1]);
    for (int y = 0; y < IMAGE_H; ++y) {
      for (int x = 0; x < IMAGE_W; ++x) {
        img.push_back(image(y, x));
      }
    }
    TinyTIFFWriter_writeImage(tif, &img.front());
    TinyTIFFWriter_close(tif);
  }
  //   /// Called after processing, validates the result and calculates info
  //   void verify_results(benchmark::State &state) {
  //     // state.SetBytesProcessed(state.iterations() * sizeof(T) * IMAGE_W *
  //     // IMAGE_H);
  //     state.SetItemsProcessed(state.iterations());
  //     // state.counters["Pixels"] = std::count(dst.begin(), dst.end(), true);
  //     // state.counters["CountM"] = table[IMAGE_W * IMAGE_H - 1].m;
  //     // state.counters["CountX"] = table[IMAGE_W * IMAGE_H - 1].x;
  //     // state.counters["CountY"] = table[IMAGE_W * IMAGE_H - 1].y;
  //   }

  af::shared<bool> destination_store;
  af::shared<T> image_store;
  af::shared<bool> mask_store;
  af::shared<double> gain_store;
  //   std::vector<char> sat_store;

  af::const_ref<T, af::c_grid<2>> src;
  af::const_ref<bool, af::c_grid<2>> mask;
  af::const_ref<double, af::c_grid<2>> gain;
  af::ref<bool, af::c_grid<2>> dst;
};

#endif
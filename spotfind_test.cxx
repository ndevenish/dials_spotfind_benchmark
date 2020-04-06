#include "gtest/gtest.h"

#include "dials/algorithms/image/threshold/local.h"

// For building arrays compatible with existing algorithms
#include <scitbx/array_family/accessors/c_grid.h>
#include <scitbx/array_family/shared.h>

namespace af = scitbx::af;
template <class T> using Data = original::Data<T>;

const size_t IMAGE_W = 4000;
const size_t IMAGE_H = 4000;

const af::tiny<int, 2> kernel_size_(3, 3);
const int min_count_ = 2;
const double threshold_ = 0.0;
const double nsig_b_ = 6.0;
const double nsig_s_ = 3.0;

/// Class to generate the sample data in a consistent way
template <typename T> class ImageSource {
public:
  ImageSource()
      : dst_store(IMAGE_W * IMAGE_H), source_store(IMAGE_W * IMAGE_H),
        mask_store(IMAGE_W * IMAGE_H) {
    sat_store.resize(sizeof(Data<double>) * IMAGE_W * IMAGE_H);

    // Generate the ref objects we will be using
    src = af::const_ref<T, af::c_grid<2>>(source_store.begin(),
                                          af::c_grid<2>(IMAGE_W, IMAGE_H));
    mask = af::const_ref<bool, af::c_grid<2>>(mask_store.begin(),
                                              af::c_grid<2>(IMAGE_W, IMAGE_H));
    dst = af::ref<bool, af::c_grid<2>>(dst_store.begin(),
                                       af::c_grid<2>(IMAGE_W, IMAGE_H));
    table = af::ref<Data<T>>((Data<T> *)&sat_store[0], sat_store.size());
    target = af::ref<unsigned char>((unsigned char *)&sat_store[0],
                                    sat_store.size());

    // Don't mask everything
    std::fill(mask_store.begin(), mask_store.end(), true);

    // Create the source image
    // src is a const_ref - we want to access it as a ref
    auto writable_src = af::ref<T, af::c_grid<2>>(
        source_store.begin(), af::c_grid<2>(IMAGE_W, IMAGE_H));

    // Create a known grid of spots, that we know the answers for

    int count = 0;
    for (int x = 0; x < IMAGE_W; x += 1) {
      for (int y = 0; y < IMAGE_W; y += 1) {
        if (y * IMAGE_W + x % 2 == 0) {
          writable_src(x, y) = 1;
        }
        // writable_src(x, y) = 1;
      }
    }

    for (int x = 0; x < IMAGE_W; x += 42) {
      for (int y = 0; y < IMAGE_W; y += 42) {
        // if (y*IMAGE_W+x % 2 == 0) {
        //   writable_src(x, y) = 1;
        // }
        writable_src(x, y) = 100;
        count += 1;
      }
    }

    BOOST_ASSERT(count == 9216);

    // Calculate the SAT - of our sample/random data, but still ensures
    // // that any basic dependencies on SAT-style data are fulfilled
    // auto dp = dials::algorithms::DispersionThreshold(
    //     af::tiny<int, 2>(IMAGE_W, IMAGE_H), kernel_size_, nsig_b_, nsig_s_,
    //     threshold_, min_count_);
    // dp.compute_sat(table, src, mask);

    benchmark::ClobberMemory();
  }

  /// Called after processing, validates the result and calculates info
  void verify_results(benchmark::State &state) {
    // state.SetBytesProcessed(state.iterations() * sizeof(T) * IMAGE_W *
    // IMAGE_H);
    state.SetItemsProcessed(state.iterations());
    // state.counters["Pixels"] = std::count(dst.begin(), dst.end(), true);
    // state.counters["CountM"] = table[IMAGE_W * IMAGE_H - 1].m;
    // state.counters["CountX"] = table[IMAGE_W * IMAGE_H - 1].x;
    // state.counters["CountY"] = table[IMAGE_W * IMAGE_H - 1].y;
  }

  af::shared<bool> dst_store;
  af::shared<T> source_store;
  af::shared<bool> mask_store;
  std::vector<char> sat_store;

  af::const_ref<T, af::c_grid<2>> src;
  af::const_ref<bool, af::c_grid<2>> mask;
  af::ref<bool, af::c_grid<2>> dst;
  af::ref<Data<T>> table;
  af::ref<unsigned char> target;
};

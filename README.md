# DIALS Spotfind Benchmarks

Benchmarking code for the dials spotfinding, and experimenting with new
improvements and algorithms.

Requires:
  - CMake
  - Google Benchmark
  - ispc present (currently required)
Optionally supported:
  - Intel ITT for VTune instrumentation

## Build

Check out with `git clone <url> --recursive`. This may give errors because
TinyTIFF seems to have a subrepo error, but it works without this. Then
build with CMake:

```
mkdir build
cd build
cmake .. -DDIALS_BUILD=<path_to_dials_build>
make
```

The variable `DIALS_BUILD` should point to the `build/` folder from your
libtbx/dials development environment.

This should build a `spotfind_bm` and a `spotfind_test` file in the build
folder. These can be run as usual (`--help` for running options).


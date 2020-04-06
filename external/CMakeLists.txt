add_subdirectory(googletest)

add_library(tinytiff STATIC TinyTIFF/tinytiffwriter.cpp TinyTIFF/tinytiffreader.cpp)

add_subdirectory(googletest)

add_library(tinytiff STATIC TinyTIFF/tinytiffwriter.cpp TinyTIFF/tinytiffreader.cpp)
target_include_directories(tinytiff INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_subdirectory(googletest)

add_library(tinytiff STATIC TinyTIFF/tinytiffwriter.cpp TinyTIFF/tinytiffreader.cpp)
set_target_properties(tinytiff PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}"
)
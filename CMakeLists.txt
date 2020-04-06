cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(spotfind_tests)
# Boost imported target and cmake_parse_arguments were added in 3.5
# 3.8 added interface targets to OpenGL

# option(USE_OPENMP yes)
# option(USE_TBB no)

# Add the included modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

set(CMAKE_CXX_STANDARD 11)

find_package(Boost REQUIRED)
find_package(Dials REQUIRED)
find_package(benchmark REQUIRED)
set(EIGEN_ROOT ${DIALS_BUILD}/../modules/eigen)
find_package(Eigen3 REQUIRED)

add_subdirectory(googletest)






# if (USE_OPENMP)
#   find_package(OpenMP REQUIRED)
#   set_property( TARGET OpenMP::OpenMP_CXX
#                 APPEND
#                 PROPERTY INTERFACE_COMPILE_DEFINITIONS HAVE_OPENMP)
# endif()

# if(USE_TBB)
#   find_package(TBB REQUIRED tbb)
#   set_property( TARGET TBB::tbb
#                 APPEND
#                 PROPERTY INTERFACE_COMPILE_DEFINITIONS HAVE_TBB )
# endif()
# add_executable(testx test.cxx)
# target_link_libraries(testx PUBLIC Dials::Dials benchmark::benchmark)


#add_executable(thresholding threshold.cxx)
#target_link_libraries(thresholding PUBLIC 
#		Dials::Dials benchmark::benchmark Eigen::Eigen)

add_custom_command(OUTPUT dispersion.o
                   COMMAND ispc ${CMAKE_SOURCE_DIR}/dispersion.ispc -o dispersion.o
                   DEPENDS dispersion.ispc)

add_subdirectory(fast_hdf5_eiger_read)


add_executable(spotfind_test spotfind_test.cxx dispersion.o)
target_link_libraries(spotfind_test PUBLIC Dials::Dials gtest_main)

add_executable(spotfind_bm spotfind_bm.cxx dispersion.o)
target_link_libraries(spotfind_bm PUBLIC Dials::Dials benchmark::benchmark)

# add_executable(sat sat/sat_main.cxx sat/sat_refactor.cxx sat/sat_original.cxx
#               sat/sat_vector.cxx)
# target_link_libraries(sat PUBLIC Dials::Dials benchmark::benchmark )

#if(TARGET OpenMP::OpenMP_CXX)
#  target_link_libraries(thresholding PRIVATE OpenMP::OpenMP_CXX)
#  target_link_libraries(sat PRIVATE OpenMP::OpenMP_CXX)
#endif()
#if(TARGET TBB::tbb)
#  target_link_libraries(thresholding PRIVATE TBB::tbb)
#endif()

#add_executable(data data.cxx)
#target_link_libraries(data PUBLIC Dials::Dials GTest::GTest benchmark::benchmark)
# GTest::Main
# 

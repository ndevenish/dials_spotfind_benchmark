
find_package(HDF5Target REQUIRED)

add_library(eiger_read SHARED
    fast_hdf5_eiger_read.c
    bitshuffle.c
    lz4.c
    bitshuffle_core.c
    bitshuffle.c
    lz4.c
    bitshuffle_core.c
    iochain.c
)
target_link_libraries(eiger_read HDF5::HL)
# h5cc -Wall -O2 -o fast_hdf5_eiger_read fast_hdf5_eiger_read.c \

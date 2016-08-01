
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	x86_64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_X64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64f32

# Prerequisites for the build:
# native-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get install g++
#
# Building/running SIMD test:
# make -f simd_make_x64.mk
# ./simd_test.x64f32

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of x86_64-linux-gnu-g++)
# sudo apt-get install clang

# For SSE1 build use (replace): RT_128=1
# For SSE4 build use (replace): RT_128=4
# For AVX1 build use (replace): RT_256=1
# For AVX2 build use (replace): RT_256=2

# Experimental 64/32-bit hybrid mode compatible with native 64-bit ABI
# is available for the original pure 32-bit ISA using 64-bit pointers,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.x64_32

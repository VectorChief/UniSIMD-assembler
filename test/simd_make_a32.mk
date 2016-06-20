
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	aarch64-linux-gnu-g++ -O3 -g -static -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a32

# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
#
# make -f simd_make_a32.mk

# The 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out an experimental 64/32-bit hybrid mode in simd_make_a64.mk

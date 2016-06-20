
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64_32

# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# Prerequisites for emulation:
# latest QEMU(-2.5) is installed or built from source and in the PATH variable.
#
# make -f simd_make_a64.mk
# qemu-aarch64 -cpu cortex-a57 simd_test.a64_32

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.

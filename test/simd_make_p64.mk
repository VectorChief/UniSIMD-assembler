
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_32

# Prerequisites for the build:
# (cross-)compiler for PowerPC is installed and in the PATH variable.
# Prerequisites for emulation:
# latest QEMU(-2.5) is built from source and in the PATH variable.
#
# make -f simd_make_p64.mk
# qemu-ppc64le -cpu POWER8 simd_test.p64_32

# For 64-bit Power(7,7+,8) VSX big-endian target use (replace):
# powerpc64-linux-gnu-g++ -DRT_ENDIAN=1
# qemu-ppc64 -cpu POWER7 simd_test.p64_32

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.

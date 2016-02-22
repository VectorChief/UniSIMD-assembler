
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	powerpc-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_P32 -DRT_128=1 -DRT_DEBUG=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p32

# Prerequisites for the build:
# (cross-)compiler for PowerPC is installed and in the PATH variable.
# Prerequisites for emulation:
# latest QEMU(-2.5) is built from source and in the PATH variable.
#
# make -f simd_make_p32.mk
# qemu-ppc -cpu G4 simd_test.p32

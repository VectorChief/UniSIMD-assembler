
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips32r2 -mmsa \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_DEBUG=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32

# Prerequisites for the build:
# (cross-)compiler for MIPS+MSA is installed and in the PATH variable.
# Prerequisites for emulation:
# latest QEMU(-2.5) is built from source and in the PATH variable.
#
# make -f simd_make_m32.mk
# qemu-mipsel -cpu P5600 simd_test.m32

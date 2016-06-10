
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_32

# Prerequisites for the build:
# (cross-)compiler for MIPS+MSA is installed and in the PATH variable.
# Prerequisites for emulation:
# QEMU 2.4.0.1.0 from imgtec.com is installed and in the PATH variable.
#
# make -f simd_make_m64.mk
# qemu-mips64el -cpu I6400 simd_test.m64_32

# The up-to-date MIPS toolchain (g++ & QEMU) can be found here:
# https://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/

# For MIPS64 big-endian target use (replace): -EB -DRT_ENDIAN=1
# qemu-mips64 -cpu I6400 simd_test.m64_32

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.

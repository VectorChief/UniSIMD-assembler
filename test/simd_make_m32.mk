
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_m32Lr5 simd_test_m32Br5

strip:
	mips-mti-linux-gnu-strip simd_test.m32?r5*

clean:
	rm simd_test.m32*


simd_test_m32Lr5:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips32r5 -mmsa -mnan=2008 \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32Lr5

simd_test_m32Br5:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips32r5 -mmsa -mnan=2008 \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32Br5


# The up-to-date MIPS toolchain (g++) can be found here:
# https://www.mips.com/develop/tools/codescape-mips-sdk/
# https://codescape.mips.com/components/toolchain/2017.10-08/downloads.html

# On Ubuntu 16.04 Live DVD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr5+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2017.10-08.for.MIPS.MTI.Linux.CentOS-5.x86_64
# is unpacked and folder mips-mti-linux-gnu/2017.10-08/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-mti-linux-gnu/2017.10-08/bin:$PATH
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_m32.mk
# qemu-mipsel -cpu P5600 simd_test.m32Lr5 -c 1
# qemu-mips   -cpu P5600 simd_test.m32Br5 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit SIMD build use (replace): RT_128=1            (30 SIMD registers)
# For 256-bit SIMD build use (replace): RT_256=1            (15 SIMD reg-pairs)

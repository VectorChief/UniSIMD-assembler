
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


# On Ubuntu (MATE) 16.04/18.04 download MIPS toolchain:
# https://www.mips.com/develop/tools/codescape-mips-sdk/
# https://codescape.mips.com/components/toolchain/2020.06-01/downloads.html
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr5+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2020.06-01.for.MIPS.MTI.Linux.CentOS-6.x86_64
# is unpacked and folder mips-mti-linux-gnu/2020.06-01/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
# PATH=/home/ubuntu-mate/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# standalone toolchain from 2020.06-01 comes with QEMU 4.1.0 for MIPS in PATH.
# sudo apt-get install qemu-user make
#
# Compiling/running SIMD test:
# make -f simd_make_m32.mk
# qemu-mipsel -cpu P5600 simd_test.m32Lr5 -c 1
# qemu-mips   -cpu P5600 simd_test.m32Br5 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ -O0 (in place of ...-g++ -O3) on MIPS32 host
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h.
# The 128-bit 15-reg targets are supported for compatibility with x86/POWER.

# For 128-bit SIMD build use (replace): RT_128=1            (30 SIMD registers)
# For 128-bit SIMD build use (replace): RT_128=4            (15 SIMD registers)
# For 256-bit SIMD build use (replace): RT_256=1            (15 SIMD reg-pairs)

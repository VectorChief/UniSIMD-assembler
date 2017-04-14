
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_m32Lr5 simd_test_m32Br5 simd_test_m32Lr6 simd_test_m32Br6

strip:
	mips-mti-linux-gnu-strip simd_test.m32?r5
	mips-img-linux-gnu-strip simd_test.m32?r6

clean:
	rm simd_test.m32*


simd_test_m32Lr5:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips32r5 -mmsa \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32Lr5

simd_test_m32Br5:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips32r5 -mmsa \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32Br5

simd_test_m32Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mips32r6 -mmsa \
        -DRT_LINUX -DRT_M32=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32Lr6

simd_test_m32Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mips32r6 -mmsa \
        -DRT_LINUX -DRT_M32=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32Br6


# The up-to-date MIPS toolchain (g++ & QEMU) can be found here:
# https://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/

# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr5+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2016.05-03.for.MIPS.MTI.Linux.CentOS-5.x86_64
# is unpacked and folder mips-mti-linux-gnu/2016.05-03/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-mti-linux-gnu/2016.05-03/bin:$PATH
# (cross-)compiler for MIPSr6+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2016.05-03.for.MIPS.IMG.Linux.CentOS-5.x86_64
# is unpacked and folder mips-img-linux-gnu/2016.05-03/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-img-linux-gnu/2016.05-03/bin:$PATH
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_m32.mk
# qemu-mipsel -cpu P5600 simd_test.m32Lr5

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 256-bit SIMD build use (replace): RT_256=1    (uses pairs of regs/ops)

# For MIPS32 Release 6 target use the following options (replace):
# mips-img-linux-gnu-g++ -mips32r6 -DRT_M32=6
# For MIPS32 Release 6 emulation use QEMU 2.5.0.2.0 from imgtec.com:
# qemu-mipsel -cpu mips32r6-generic simd_test.m32Lr6

# For big-endian MIPS32 (r5 and r6) use (replace): -EB -DRT_ENDIAN=1
# qemu-mips -cpu *** simd_test.m32Br*
# where *** is P5600 for r5 build and mips32r6-generic for r6 build.

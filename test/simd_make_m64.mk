
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: build_le build_be

strip:
	mips-mti-linux-gnu-strip simd_test.m64???Lr6
	mips-mti-linux-gnu-strip simd_test.m64???Br6

clean:
	rm simd_test.m64*


build_le: simd_test_m64_32Lr6 simd_test_m64_64Lr6 \
          simd_test_m64f32Lr6 simd_test_m64f64Lr6

simd_test_m64_32Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_32Lr6

simd_test_m64_64Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_64Lr6

simd_test_m64f32Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f32Lr6

simd_test_m64f64Lr6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EL -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f64Lr6


build_be: simd_test_m64_32Br6 simd_test_m64_64Br6 \
          simd_test_m64f32Br6 simd_test_m64f64Br6

simd_test_m64_32Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_32Br6

simd_test_m64_64Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_64Br6

simd_test_m64f32Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f32Br6

simd_test_m64f64Br6:
	mips-mti-linux-gnu-g++ -O3 -g -static -EB -mips64r6 -mmsa -mabi=64 \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f64Br6


# On Ubuntu (MATE) 16.04-22.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
# (Ubuntu MATE is set up for an update without a need to edit the file)
# (extended repositories "universe multiverse" are only needed for clang)
#
# Download and unpack MIPS toolchain:
# https://codescape.mips.com/components/toolchain/2020.06-01/downloads.html
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr6+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2020.06-01.for.MIPS.MTI.Linux.CentOS-6.x86_64
# is unpacked and folder mips-mti-linux-gnu/2020.06-01/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
# PATH=/home/ubuntu-mate/Downloads/mips-mti-linux-gnu/2020.06-01/bin:$PATH
#
# Starting from Ubuntu (MATE) 19.10 upstream (cross-)compiler supports MSA.
# sudo apt-get install make g++-mipsisa64r6el-linux-gnuabi64
# sudo apt-get install make g++-mipsisa64r6-linux-gnuabi64
# (replace mips-mti-linux-gnu with mipsisa64r6el-linux-gnuabi64 for LE)
# (replace mips-mti-linux-gnu with mipsisa64r6-linux-gnuabi64 for BE)
#
# Prerequisites for emulation:
# recent QEMU(-2.7) is installed or built from source and in the PATH variable.
# standalone toolchain from 2020.06-01 comes with QEMU 4.1.0 for MIPS in PATH.
# sudo apt-get install qemu-user make
#
# Compiling/running SIMD test:
# make -f simd_make_m64.mk
# qemu-mips64el -cpu I6400 simd_test.m64_32Lr6 -c 1
# qemu-mips64el -cpu I6400 simd_test.m64_64Lr6 -c 1
# qemu-mips64el -cpu I6400 simd_test.m64f32Lr6 -c 1
# qemu-mips64el -cpu I6400 simd_test.m64f64Lr6 -c 1
# qemu-mips64   -cpu I6400 simd_test.m64_32Br6 -c 1
# qemu-mips64   -cpu I6400 simd_test.m64_64Br6 -c 1
# qemu-mips64   -cpu I6400 simd_test.m64f32Br6 -c 1
# qemu-mips64   -cpu I6400 simd_test.m64f64Br6 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ -O0 (in place of ...-g++ -O3) on MIPS64r6 host (I6400/P6600)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h.
# The 128-bit 15-reg targets are supported for compatibility with x86/POWER.

# For 128-bit SIMD build use (replace): RT_128=1            (30 SIMD registers)
# For 128-bit SIMD build use (replace): RT_128=4            (15 SIMD registers)
# For 256-bit SIMD build use (replace): RT_256=1            (15 SIMD reg-pairs)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.m64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.m64*64

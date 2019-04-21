
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: build_le build_be

strip:
	mips-img-linux-gnu-strip simd_test.m64*

clean:
	rm simd_test.m64*


build_le: simd_test_m64_32Lr6 simd_test_m64_64Lr6 \
          simd_test_m64f32Lr6 simd_test_m64f64Lr6

simd_test_m64_32Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_32Lr6

simd_test_m64_64Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_64Lr6

simd_test_m64f32Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f32Lr6

simd_test_m64f64Lr6:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f64Lr6


build_be: simd_test_m64_32Br6 simd_test_m64_64Br6 \
          simd_test_m64f32Br6 simd_test_m64f64Br6

simd_test_m64_32Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_32Br6

simd_test_m64_64Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_64Br6

simd_test_m64f32Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f32Br6

simd_test_m64f64Br6:
	mips-img-linux-gnu-g++ -O3 -g -static -EB -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f64Br6


# The up-to-date MIPS toolchain (g++) can be found here:
# https://www.mips.com/develop/tools/codescape-mips-sdk/
# https://codescape.mips.com/components/toolchain/2017.10-08/downloads.html
#
# Prerequisites for the build:
# (cross-)compiler for MIPSr6+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2017.10-08.for.MIPS.IMG.Linux.CentOS-5.x86_64
# is unpacked and folder mips-img-linux-gnu/2017.10-08/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-img-linux-gnu/2017.10-08/bin:$PATH
# PATH=/home/ubuntu-mate/Downloads/mips-img-linux-gnu/2017.10-08/bin:$PATH
#
# Prerequisites for emulation:
# recent QEMU(-2.7) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu-user make
#
# Building/running SIMD test:
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
# clang++ -O0 (in place of ...-g++ -O3) on MIPS64 host
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

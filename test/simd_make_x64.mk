
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: build_x64 build_avx

strip:
	strip simd_test.x64*

clean:
	rm simd_test.x64*


build_x64: simd_test_x64_32 simd_test_x64_64 simd_test_x64f32 simd_test_x64f64

simd_test_x64_32:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_128=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64_32

simd_test_x64_64:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_128=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64_64

simd_test_x64f32:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_256_R8=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64f32

simd_test_x64f64:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_256_R8=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64f64


build_avx: simd_test_x64_32avx simd_test_x64_64avx \
           simd_test_x64f32avx simd_test_x64f64avx

simd_test_x64_32avx:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_256=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64_32avx

simd_test_x64_64avx:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_256=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64_64avx

simd_test_x64f32avx:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_512=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64f32avx

simd_test_x64f64avx:
	g++ -O3 -g \
        -DRT_LINUX -DRT_X64 -DRT_512=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x64f64avx


# Prerequisites for the build:
# native-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get install g++
#
# Prerequisites for emulation:
# http://software.intel.com/en-us/articles/intel-software-development-emulator
# Intel SDE is downloaded, unpacked and in the PATH variable.
#
# Building/running SIMD test:
# make -f simd_make_x64.mk
# ./simd_test.x64f32
# sde64 -hsw -- ./simd_test.x64_32avx -c 1
# sde64 -skx -- ./simd_test.x64f32avx -c 1
# Use "-c 1" option to reduce test time when emulating with Intel SDE

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of g++)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit 30-reg build use (replace): RT_128=1   (reserved for AVX1+2/SSEx)
# For 128-bit 30-reg build use (replace): RT_128=2   (Skylake-X /w AVX512DQ+VL)
# For 128-bit SSE2 build use (replace): RT_128=4 RT_SIMD_COMPAT_SSE=2
# For 128-bit SSE4 build use (replace): RT_128=4            (15 SIMD registers)
# For 128-bit AVX1 build use (replace): RT_128=8            (15 SIMD registers)
# For 128-bit FMA3 build use (replace): RT_128=16           (AMD's AVX1+FMA3)
# For 128-bit AVX2 build use (replace): RT_128=32           (AMD's AVX2+FMA3)

# For 256-bit SSE2 build use (replace): RT_256_R8=4 RT_SIMD_COMPAT_SSE=2
# For 256-bit SSE4 build use (replace): RT_256_R8=4          (8 SIMD reg-pairs)
# For 256-bit AVX1 build use (replace): RT_256=1            (15 SIMD registers)
# For 256-bit AVX2 build use (replace): RT_256=2            (15 SIMD registers)
# For 256-bit 30-reg build use (replace): RT_256=4   (reserved for AVX1+2/SSEx)
# For 256-bit 30-reg build use (replace): RT_256=8   (Skylake-X /w AVX512DQ+VL)

# For 512-bit AVX1 build use (replace): RT_512_R8=1          (8 SIMD reg-pairs)
# For 512-bit AVX2 build use (replace): RT_512_R8=2          (8 SIMD reg-pairs)
# For 512-bit AVX512F  build use (replace): RT_512=1        (15 SIMD registers)
# For 512-bit AVX512DQ build use (replace): RT_512=2        (15 SIMD registers)
# For 512-bit AVX512F  build use (replace): RT_512=4        (30 SIMD registers)
# For 512-bit AVX512DQ build use (replace): RT_512=8        (30 SIMD registers)

# For 1024-bit AVX512F  build use (replace): RT_1K4=1       (15 SIMD reg-pairs)
# For 1024-bit AVX512DQ build use (replace): RT_1K4=2       (15 SIMD reg-pairs)
# For 2048-bit AVX512F  build use (replace): RT_2K8_R8=1     (8 SIMD reg-quads)
# For 2048-bit AVX512DQ build use (replace): RT_2K8_R8=2     (8 SIMD reg-quads)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.x64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.x64*64

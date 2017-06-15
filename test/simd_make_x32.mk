
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_x32

strip:
	x86_64-linux-gnu-strip simd_test.x32

clean:
	rm simd_test.x32


simd_test_x32:
	x86_64-linux-gnu-g++ -O3 -g -static -mx32 \
        -DRT_LINUX -DRT_X32 -DRT_256_R8=4 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x32


# Prerequisites for the build:
# multilib-compiler for x86_64 is installed and in the PATH variable.
# sudo apt-get install g++-multilib
# (installation of g++-multilib removes any g++ cross-compilers)
#
# Building/running SIMD test:
# make -f simd_make_x32.mk
# ./simd_test.x32

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of x86_64-linux-gnu-g++)
# sudo apt-get install clang (requires g++-multilib for non-native ABI)

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit AVX3.2 build use (replace): RT_128=2 (Skylake X and AVX-512++)
# For 128-bit SSE2 build use (replace): RT_128=4 RT_SIMD_COMPAT_SSE=2
# For 128-bit SSE4 build use (replace): RT_128=4
# For 128-bit AVX1 build use (replace): RT_128=8
# For 128-bit AVX2 build use (replace): RT_128=8 RT_SIMD_COMPAT_128=2 (FMA3)
# For 128-bit FMA3 build use (replace): RT_128=8 RT_SIMD_COMPAT_128=3 (FMA3)
# For 128-bit FMA3 build use (replace): RT_128=16       (as above AVX1+FMA3)

# For 256-bit SSE2 build use (replace): RT_256_R8=4 RT_SIMD_COMPAT_SSE=2
# For 256-bit SSE4 build use (replace): RT_256_R8=4       (8 SIMD-reg pairs)
# For 256-bit AVX1 build use (replace): RT_256=1
# For 256-bit AVX2 build use (replace): RT_256=2                      (FMA3)
# For 256-bit FMA3 build use (replace): RT_256=4                 (AVX1+FMA3)
# For 256-bit AVX3.2 build use (replace): RT_256=8 (Skylake X and AVX-512++)

# For 512-bit AVX1 build use (replace): RT_512_R8=1       (8 SIMD-reg pairs)
# For 512-bit AVX2 build use (replace): RT_512_R8=2       (8 SIMD-reg pairs)
# For 512-bit AVX3.1 build use (replace): RT_512=1
# For 512-bit AVX3.2 build use (replace): RT_512=2

# For 1024-bit AVX3.1 build use (replace): RT_1K4=1
# For 1024-bit AVX3.2 build use (replace): RT_1K4=2
# For 2048-bit AVX3.1 build use (replace): RT_2K8_R8=1    (8 SIMD-reg quads)
# For 2048-bit AVX3.2 build use (replace): RT_2K8_R8=2    (8 SIMD-reg quads)

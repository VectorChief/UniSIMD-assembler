
INC_PATH =                              \
        -I../core/

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
        -DRT_LINUX -DRT_X32 -DRT_128=2 -DRT_DEBUG=0 \
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

# For 128-bit SSE1 build use (replace): RT_128=1
# For 128-bit SSE4 build use (replace): RT_128=4

# For 128-bit AVX1 build use (replace): RT_128=8
# For 128-bit AVX2 build use (replace): RT_128=8, RT_SIMD_COMPAT_128=2 in rtarch.h

# For 256-bit AVX1 build use (replace): RT_256=1
# For 256-bit AVX2 build use (replace): RT_256=2

# For 512-bit AVX3.1 build use (replace): RT_512=1
# For 512-bit AVX3.2 build use (replace): RT_512=2

# For generic BASE X32 build keep: RT_X32 (default)
# For 3-op-VEX BASE X32 build use (replace): RT_X32=1 (reserved)
# For BMI1+BMI2 BASE X32 build use (replace): RT_X32=2


INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_a64_32 simd_test_a64_64 simd_test_a64f32 simd_test_a64f64

strip:
	aarch64-linux-gnu-strip simd_test.a64*

clean:
	rm simd_test.a64*


simd_test_a64_32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64_32

simd_test_a64_64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64_64

simd_test_a64f32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64f32

simd_test_a64f64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64f64


# On Ubuntu 16.04 Live DVD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install g++-aarch64-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_a64.mk
# qemu-aarch64 -cpu cortex-a57 simd_test.a64f32

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of ...-g++)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit NEON build use (replace): RT_128=1    (uses 30 SIMD registers)
# For 256-bit NEON build use (replace): RT_256=1    (uses 15 SIMD reg-pairs)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.a64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.a64*64

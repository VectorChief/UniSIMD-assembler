
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: build_le build_be

strip:
	powerpc64le-linux-gnu-strip simd_test.p64???L*
	powerpc64-linux-gnu-strip simd_test.p64???B*

clean:
	rm simd_test.p64*


build_le: simd_test_p64_32Lp8 simd_test_p64_64Lp8 simd_test_p64f32Lp8 simd_test_p64f64Lp8

simd_test_p64_32Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_32Lp8

simd_test_p64_64Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_64Lp8

simd_test_p64f32Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f32Lp8

simd_test_p64f64Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f64Lp8


build_be: simd_test_p64_32Bp7 simd_test_p64_64Bp7 simd_test_p64f32Bp7 simd_test_p64f64Bp7

simd_test_p64_32Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_32Bp7

simd_test_p64_64Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_64Bp7

simd_test_p64f32Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_512=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f32Bp7

simd_test_p64f64Bp7:
	powerpc64-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_512=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f64Bp7


# On Ubuntu 16.04 Live DVD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for 64-bit Power is installed and in the PATH variable.
# sudo apt-get install g++-powerpc64le-linux-gnu
# (recent g++-5-powerpc64le series target POWER8 and don't work well with -O3)
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_p64.mk
# qemu-ppc64le -cpu POWER8 simd_test.p64f32Lp8 -c 1000
# qemu-ppc64 -cpu POWER7 simd_test.p64f32Bp7 -c 1000
# Use "-c 1000" option to reduce test time when emulating with QEMU

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit VSX1 build use (replace): RT_128=1    (uses 30 SIMD registers)
# For 128-bit VSX2 build use (replace): RT_128=2    (uses 30 SIMD registers)
# For 128-bit VMX  build use (replace): RT_128=4    (uses 15 SIMD registers)

# For 256-bit VMX  build use (replace): RT_256_R8=4 (uses  8 SIMD reg-pairs)
# For 256-bit VSX1 build use (replace): RT_256=1    (uses 15 SIMD reg-pairs)
# For 256-bit VSX2 build use (replace): RT_256=2    (uses 15 SIMD reg-pairs)
# For 256-bit VSX1 build use (replace): RT_256=4    (uses 30 SIMD reg-pairs)
# For 256-bit VSX2 build use (replace): RT_256=8    (uses 30 SIMD reg-pairs)

# For 512-bit VSX1 build use (replace): RT_512=1    (uses 15 SIMD reg-quads)
# For 512-bit VSX2 build use (replace): RT_512=2    (uses 15 SIMD reg-quads)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.p64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.p64*64

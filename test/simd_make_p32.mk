
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_p32Bg4 simd_test_p32Bp7 simd_test_p32Bp8

strip:
	powerpc-linux-gnu-strip simd_test.p32*

clean:
	rm simd_test.p32*


simd_test_p32Bg4:
	powerpc-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_P32 -DRT_128=4 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p32Bg4

simd_test_p32Bp7:
	powerpc-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_P32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p32Bp7

simd_test_p32Bp8:
	powerpc-linux-gnu-g++ -O3 -g -static -DRT_SIMD_COMPAT_PW8=1 \
        -DRT_LINUX -DRT_P32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p32Bp8


# On Ubuntu 16.04 Live DVD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for PowerPC is installed and in the PATH variable.
# sudo apt-get install g++-powerpc-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_p32.mk
# qemu-ppc        -cpu G4     simd_test.p32Bg4 -c 1
# qemu-ppc64abi32 -cpu POWER7 simd_test.p32Bp7 -c 1
# qemu-ppc64abi32 -cpu POWER8 simd_test.p32Bp8 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit VSX1 build use (replace): RT_128=1    (uses 30 SIMD registers)
# For 128-bit VSX2 build use (replace): RT_128=1 RT_SIMD_COMPAT_PW8=1
# For 128-bit VMX  build use (replace): RT_128=4    (uses 15 SIMD registers)

# For 256-bit VMX  build use (replace): RT_256_R8=4 (uses  8 SIMD reg-pairs)
# For 256-bit VSX1 build use (replace): RT_256=1    (uses 15 SIMD reg-pairs)
# For 256-bit VSX2 build use (replace): RT_256=1 RT_SIMD_COMPAT_PW8=1
# For 256-bit VSX1 build use (replace): RT_256=4    (uses 30 SIMD reg-pairs)
# For 256-bit VSX2 build use (replace): RT_256=4 RT_SIMD_COMPAT_PW8=1

# For 512-bit VSX1 build use (replace): RT_512=1    (uses 15 SIMD reg-quads)
# For 512-bit VSX2 build use (replace): RT_512=1 RT_SIMD_COMPAT_PW8=1

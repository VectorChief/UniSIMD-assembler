
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_p32Bg4 simd_test_p32Bp7 simd_test_p32Bp8 simd_test_p32Bp9

strip:
	powerpc-linux-gnu-strip simd_test.p32*

clean:
	rm simd_test.p32*


simd_test_p32Bg4:
	powerpc-linux-gnu-g++ -O3 -g -static -DRT_SIMD_COMPAT_VSX=0 \
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

simd_test_p32Bp9:
	powerpc-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_P32 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p32Bp9


# On Ubuntu (MATE) 16.04-22.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
# (Ubuntu MATE is set up for an update without a need to edit the file)
# (extended repositories "universe multiverse" are only needed for clang)
#
# Prerequisites for the build:
# (cross-)compiler for PowerPC is installed and in the PATH variable.
# sudo apt-get install make g++-powerpc-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# POWER9 target requires more recent QEMU, tested with 3.x.y series and 4.2.0.
# QEMU versions 4.x.y prior to 4.2.0 show issues with POWER8/9 fp32 LE targets.
# sudo apt-get install qemu-user
#
# Compiling/running SIMD test:
# make -f simd_make_p32.mk
# qemu-ppc        -cpu G4     simd_test.p32Bg4 -c 1
# qemu-ppc64abi32 -cpu POWER7 simd_test.p32Bp7 -c 1
# qemu-ppc64abi32 -cpu POWER8 simd_test.p32Bp8 -c 1
# qemu-ppc64abi32 -cpu POWER9 simd_test.p32Bp9 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ -O0 (in place of ...-g++ -O3) on PowerPC host (G4)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h.
# The RT_SIMD_COMPAT_PW8=1 flag below is redundant when building in LE mode.

# For 128-bit VSX1 build use (replace): RT_128=1            (30 SIMD registers)
# For 128-bit VSX2 build use (replace): RT_128=1 RT_SIMD_COMPAT_PW8=1 (30 regs)
# For 128-bit VSX3 build use (replace): RT_128=2            (30 SIMD registers)
# For 128-bit VMX  build use (replace): RT_128=4 RT_SIMD_COMPAT_VSX=0 (15 regs)

# For 256-bit VMX  build use (replace): RT_256_R8=4 RT_SIMD_COMPAT_VSX=0 (8 rp)
# For 256-bit VSX1 build use (replace): RT_256=1            (15 SIMD reg-pairs)
# For 256-bit VSX2 build use (replace): RT_256=1 RT_SIMD_COMPAT_PW8=1   (15 rp)
# For 256-bit VSX3 build use (replace): RT_256=2            (15 SIMD reg-pairs)
# For 256-bit VSX1 build use (replace): RT_256=4 (<=test29) (30 SIMD reg-pairs)
# For 256-bit VSX2 build use (replace): RT_256=4 RT_SIMD_COMPAT_PW8=1   (30 rp)
# For 256-bit VSX3 build use (replace): RT_256=8 (<=test29) (30 SIMD reg-pairs)

# For 512-bit VSX1 build use (replace): RT_512=1 (<=test29) (15 SIMD reg-quads)
# For 512-bit VSX2 build use (replace): RT_512=1 RT_SIMD_COMPAT_PW8=1   (15 rq)
# For 512-bit VSX3 build use (replace): RT_512=2 (<=test29) (15 SIMD reg-quads)

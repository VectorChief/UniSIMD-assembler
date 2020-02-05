
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: build_p9 build_pX build_le build_be

strip:
	powerpc64le-linux-gnu-strip simd_test.p64???L*
	powerpc64-linux-gnu-strip simd_test.p64???B*

clean:
	rm simd_test.p64*


build_p9: simd_test_p64_32Lp9 simd_test_p64_64Lp9 \
          simd_test_p64f32Lp9 simd_test_p64f64Lp9

simd_test_p64_32Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_32Lp9

simd_test_p64_64Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_64Lp9

simd_test_p64f32Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f32Lp9

simd_test_p64f64Lp9:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f64Lp9


build_pX: simd_test_p64_32LpX simd_test_p64_64LpX \
          simd_test_p64f32LpX simd_test_p64f64LpX

simd_test_p64_32LpX:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=8 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_32LpX

simd_test_p64_64LpX:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=8 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_64LpX

simd_test_p64f32LpX:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_512=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f32LpX

simd_test_p64f64LpX:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_512=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f64LpX


build_le: simd_test_p64_32Lp8 simd_test_p64_64Lp8 \
          simd_test_p64f32Lp8 simd_test_p64f64Lp8

simd_test_p64_32Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_32Lp8

simd_test_p64_64Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_64Lp8

simd_test_p64f32Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f32Lp8

simd_test_p64f64Lp8:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64f64Lp8


build_be: simd_test_p64_32Bp7 simd_test_p64_64Bp7 \
          simd_test_p64f32Bp7 simd_test_p64f64Bp7

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


# On Ubuntu (MATE) 16.04/18.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
#
# Prerequisites for the build:
# (cross-)compiler for 64-bit POWER is installed and in the PATH variable.
# sudo apt-get install make g++-powerpc64le-linux-gnu
# sudo apt-get install make g++-powerpc64-linux-gnu
# (recent g++-5-powerpc64le series target POWER8 and don't work well with -O3)
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# POWER9 target requires more recent QEMU, tested with 3.x.y series and 4.2.0.
# QEMU versions 4.x.y prior to 4.2.0 show issues with POWER8/9 fp32 LE targets.
# sudo apt-get install qemu-user
#
# Compiling/running SIMD test:
# make -f simd_make_p64.mk
# qemu-ppc64le -cpu POWER9 simd_test.p64_32Lp9 -c 1
# qemu-ppc64le -cpu POWER9 simd_test.p64_64Lp9 -c 1
# qemu-ppc64le -cpu POWER9 simd_test.p64f32Lp9 -c 1
# qemu-ppc64le -cpu POWER9 simd_test.p64f64Lp9 -c 1
# qemu-ppc64le -cpu POWER9 simd_test.p64_32LpX -c 1
# qemu-ppc64le -cpu POWER9 simd_test.p64_64LpX -c 1
# qemu-ppc64le -cpu POWER9 simd_test.p64f32LpX -c 1
# qemu-ppc64le -cpu POWER9 simd_test.p64f64LpX -c 1
# qemu-ppc64le -cpu POWER8 simd_test.p64_32Lp8 -c 1
# qemu-ppc64le -cpu POWER8 simd_test.p64_64Lp8 -c 1
# qemu-ppc64le -cpu POWER8 simd_test.p64f32Lp8 -c 1
# qemu-ppc64le -cpu POWER8 simd_test.p64f64Lp8 -c 1
# qemu-ppc64   -cpu POWER7 simd_test.p64_32Bp7 -c 1
# qemu-ppc64   -cpu POWER7 simd_test.p64_64Bp7 -c 1
# qemu-ppc64   -cpu POWER7 simd_test.p64f32Bp7 -c 1
# qemu-ppc64   -cpu POWER7 simd_test.p64f64Bp7 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build works too (takes much longer prior to 3.8), use (replace):
# clang++ -O0 (in place of ...-g++ -O2) on 64-bit POWER host (Tyan TN71-BP012)
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
# For 256-bit VSX1 build use (replace): RT_256=4            (30 SIMD reg-pairs)
# For 256-bit VSX2 build use (replace): RT_256=4 RT_SIMD_COMPAT_PW8=1   (30 rp)
# For 256-bit VSX3 build use (replace): RT_256=8            (30 SIMD reg-pairs)

# For 512-bit VSX1 build use (replace): RT_512=1            (15 SIMD reg-quads)
# For 512-bit VSX2 build use (replace): RT_512=1 RT_SIMD_COMPAT_PW8=1   (15 rq)
# For 512-bit VSX3 build use (replace): RT_512=2            (15 SIMD reg-quads)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.p64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.p64*64

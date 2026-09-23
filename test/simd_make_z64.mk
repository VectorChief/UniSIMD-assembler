
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: build_z64

strip:
	s390x-linux-gnu-strip simd_test.z64*

clean:
	rm simd_test.z64*


build_z64: simd_test_z64_32 simd_test_z64_64 \
           simd_test_z64f32 simd_test_z64f64

simd_test_z64_32:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.z64_32

simd_test_z64_64:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.z64_64

simd_test_z64f32:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.z64f32

simd_test_z64f64:
	s390x-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_Z64 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.z64f64


# On Ubuntu (MATE) 16.04-26.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
# (Ubuntu MATE is set up for an update without a need to edit the file)
# (extended repositories "universe multiverse" are only needed for clang)
#
# Prerequisites for the build:
# (cross-)compiler for s390x is installed and in the PATH variable.
# in simd_test.cpp set SUB_TEST to 23
# sudo apt-get install make g++-s390x-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-8.2) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu-user
#
# Compiling/running SIMD test:
# make -f simd_make_z64.mk
# qemu-s390x    -cpu max simd_test.z64_32 -c 1
# qemu-s390x    -cpu max simd_test.z64_64 -c 1
# qemu-s390x    -cpu max simd_test.z64f32 -c 1
# qemu-s390x    -cpu max simd_test.z64f64 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ -O0 (in place of ...-g++ -O3) on s390x host (z13/z14)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h.
# The 128-bit 15-reg targets are supported for compatibility with x86/POWER.

# For 128-bit SIMD build use (replace): RT_128=1            (30 SIMD registers)
# For 128-bit SIMD build use (replace): RT_128=4            (15 SIMD registers)
# For 256-bit SIMD build use (replace): RT_256=1            (15 SIMD reg-pairs)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.z64_**
# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.z64*64

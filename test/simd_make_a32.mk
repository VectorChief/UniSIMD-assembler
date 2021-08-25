
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_a32

strip:
	aarch64-linux-gnu-strip simd_test.a32*

clean:
	rm simd_test.a32*


simd_test_a32:
	aarch64-linux-gnu-g++ -O3 -g -static -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a32


# On Ubuntu (MATE) 16.04-20.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
# (Ubuntu MATE is set up for an update without a need to edit the file)
# (extended repositories "universe multiverse" are only needed for clang)
#
# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install make g++-aarch64-linux-gnu
# (recent upstream g++-5-aarch64 series may not fully support ILP32 ABI)
#
# Compiling/running SIMD test:
# make -f simd_make_a32.mk

# Clang native build should theoretically work too (not tested), use (replace):
# clang++ (in place of ...-g++) on AArch64 host (Raspberry Pi 3/4)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h.
# The 128-bit 15-reg targets are supported for compatibility with x86/POWER.

# For 128-bit NEON build use (replace): RT_128=1            (30 SIMD registers)
# For 128-bit ARMv8.2 build use (replace): RT_128=2 (adds new fp16 ops) (30 rs)
# For 128-bit NEON build use (replace): RT_128=4            (15 SIMD registers)
# For 128-bit ARMv8.2 build use (replace): RT_128=8 (adds new fp16 ops) (15 rs)
# For 256-bit NEON build use (replace): RT_256=1            (15 SIMD reg-pairs)
# For 256-bit ARMv8.2 build use (replace): RT_256=2 (adds new fp16 ops) (15 rp)

# For 256-bit  SVEx1 build use (replace): RT_256=4          (30 SIMD registers)
# For 512-bit  SVEx2 build use (replace): RT_512=1          (15 SIMD reg-pairs)
# For 512-bit  SVEx1 build use (replace): RT_512=4          (30 SIMD registers)
# For 1024-bit SVEx2 build use (replace): RT_1K4=1          (15 SIMD reg-pairs)
# For 1024-bit SVEx1 build use (replace): RT_1K4=4          (30 SIMD registers)
# For 2048-bit SVEx2 build use (replace): RT_2K8_R8=1        (8 SIMD reg-pairs)
# For 2048-bit SVEx1 build use (replace): RT_2K8_R8=4       (15 SIMD registers)
# The last two slots are artificially reg-limited for compatibility with AVX512

# 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out 64/32-bit (ptr/adr) hybrid mode for 64-bit ABI in simd_make_a64.mk

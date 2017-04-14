
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_a32

strip:
	aarch64-linux-gnu-strip simd_test.a32

clean:
	rm simd_test.a32


simd_test_a32:
	aarch64-linux-gnu-g++ -O3 -g -static -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a32


# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install g++-aarch64-linux-gnu
# (recent upstream g++-5-aarch64 series may not fully support ILP32 ABI)
#
# Building/running SIMD test:
# make -f simd_make_a32.mk
# (recent upstream QEMU(-2.6) doesn't currently support AArch64 ILP32 ABI)

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of ...-g++)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 256-bit NEON build use (replace): RT_256=1    (uses pairs of regs/ops)

# 32-bit ABI hasn't been fully tested yet due to lack of available libs,
# check out 64/32-bit (ptr/adr) hybrid mode for 64-bit ABI in simd_make_a64.mk

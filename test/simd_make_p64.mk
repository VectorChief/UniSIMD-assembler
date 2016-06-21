
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	powerpc64le-linux-gnu-g++ -O2 -g -static \
        -DRT_LINUX -DRT_P64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.p64_32

# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for 64-bit Power is installed and in the PATH variable.
# sudo apt-get install g++-powerpc64le-linux-gnu
#
# Prerequisites for emulation:
# latest QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# (recent g++-5-powerpc64le series target POWER8 and don't work well with -O3)
# make -f simd_make_p64.mk
# qemu-ppc64le -cpu POWER8 simd_test.p64_32

# For 64-bit Power(7,7+,8) VSX big-endian target use (replace):
# powerpc64-linux-gnu-g++ -O3 -DRT_ENDIAN=1
# qemu-ppc64 -cpu POWER7 simd_test.p64_32

# Experimental 64/32-bit hybrid mode is enabled by default
# until full 64-bit support is implemented in the framework.

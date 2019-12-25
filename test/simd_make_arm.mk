
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm

# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for ARMv7 is installed and in the PATH variable.
# sudo apt-get install g++-arm-linux-gnueabi
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_arm.mk
# qemu-arm -cpu cortex-a8 simd_test.arm

# 0) Build flags above are intended for default "vanilla" ARMv7 target, while
# settings suitable for specific hardware platforms are given below (replace).
# 1) Nokia N900, Maemo 5 scratchbox: "vanilla"
# 2) Raspberry Pi 2, Raspbian: arm-linux-gnueabihf-g++ -DRT_128=2
# 3) Raspberry Pi 3, Raspbian: arm-linux-gnueabihf-g++ -DRT_128=4

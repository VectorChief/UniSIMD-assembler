
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_arm_v1 simd_test_arm_v2

strip:
	arm-linux-gnueabi-strip simd_test.arm_v*

clean:
	rm simd_test.arm_v*


simd_test_arm_v1:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_v1

simd_test_arm_v2:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_v2


build_n900: simd_test_arm_n900

strip_n900:
	arm-linux-gnueabi-strip simd_test.arm_n900*

clean_n900:
	rm simd_test.arm_n900*


simd_test_arm_n900:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_n900


build_rpiX: simd_test_arm_rpi2 simd_test_arm_rpi3

strip_rpiX:
	arm-linux-gnueabihf-strip simd_test.arm_rpi*

clean_rpiX:
	rm simd_test.arm_rpi*


simd_test_arm_rpi2:
	arm-linux-gnueabihf-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_rpi2

simd_test_arm_rpi3:
	arm-linux-gnueabihf-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=4 -DRT_DEBUG=0 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm_rpi3


# On Ubuntu (Mate) 16.04/18.04 add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo nano /etc/apt/sources.list) then run:
# sudo apt-get update
#
# Prerequisites for the build:
# (cross-)compiler for ARMv7 is installed and in the PATH variable.
# sudo apt-get install make g++-arm-linux-gnueabi
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# sudo apt-get install qemu-user
#
# Building/running SIMD test:
# make -f simd_make_arm.mk
# qemu-arm -cpu cortex-a8  simd_test.arm_v1 -c 1
# qemu-arm -cpu cortex-a15 simd_test.arm_v2 -c 1
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang native build works too (takes much longer prior to 3.8), use (replace):
# clang++ -O0 (in place of ...-g++ -O3) on ARMv7 host (Raspberry Pi 2)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h.
# Original legacy 32-bit ARMv7/x86 targets only support 8 SIMD registers.

# 1) Nokia N900, Maemo 5 scratchbox: "vanilla" (-DRT_128=1)  (8 SIMD registers)
# 2) Raspberry Pi 2, Raspbian: arm-linux-gnueabihf-g++ -DRT_128=2 (8 SIMD regs)
# 3) Raspberry Pi 3, Raspbian: arm-linux-gnueabihf-g++ -DRT_128=4 (8 SIMD regs)

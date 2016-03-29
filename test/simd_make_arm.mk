
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	arm-linux-gnueabi-g++ -O3 -g -static \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -DRT_DEBUG=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm

# Prerequisites for the build:
# (cross-)compiler for ARMv7 is installed and in the PATH variable.
# Prerequisites for emulation:
# latest QEMU(-2.5) is built from source and in the PATH variable.
#
# make -f simd_make_arm.mk
# qemu-arm -cpu cortex-a8 simd_test.arm

# 0) Build flags above are intended for default "vanilla" ARMv7 target, while
# settings suitable for specific hardware platform are given below (replace).
# 1) Nokia N900, Maemo 5 scratchbox: "vanilla"
# 2) Raspberry Pi 2, Raspbian: arm-linux-gnueabihf-g++ -DRT_128=2

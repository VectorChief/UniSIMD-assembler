
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
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm

# 0) Build flags above are intended for default "vanilla" ARMv7 target, while
# settings suitable for specific hardware platform are given below (replace).
# 1) Nokia N900, Maemo 5 scratchbox: "vanilla"
# 2) Raspberry Pi 2, Raspbian: arm-linux-gnueabihf-g++ -DRT_ARM=2

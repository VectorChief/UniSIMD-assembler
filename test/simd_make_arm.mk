
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	arm-linux-gnueabi-g++ -O3 -g -static -march=armv7-a -marm \
        -DRT_LINUX -DRT_ARM -DRT_128=1 -DRT_DEBUG=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.arm

# Build target above is suitable for Maemo/N900 and DEB-based cross-compilation.
# Other (than DEB-based) Linux systems may have different name for the compiler.
# For native builds on ARMv7 (Raspberry Pi 2) use plain g++ reference.

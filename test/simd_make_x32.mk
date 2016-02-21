
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	x86_64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_X32 -DRT_128=2 -DRT_DEBUG=1 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.x32

# For actual x32 target use: -mx32, -DRT_POINTER=32 (replace).
# Experimental 64/32-bit hybrid support is enabled for compatibility
# with wider spectrum of toolchains/libraries in the standard x64 target.

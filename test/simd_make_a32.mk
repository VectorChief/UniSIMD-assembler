
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	aarch64-linux-gnu-g++ -O3 -g -static -mabi=ilp32 \
        -DRT_LINUX -DRT_A32 -DRT_128=1 -DRT_DEBUG=1 \
        -DRT_POINTER=32 -DRT_ADDRESS=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a32

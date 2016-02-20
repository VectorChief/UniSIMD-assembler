
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	mips-mti-linux-gnu-g++ -O3 -g -EL -mips32r2 -mmsa \
        -DRT_LINUX -DRT_M32 -DRT_128=1 -DRT_DEBUG=1 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m32

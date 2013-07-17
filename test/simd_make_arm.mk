INC_PATH =                          \
        -I../core/

SRC_LIST =                          \
        simd_test.cpp

simd_test:
	g++ -O3 -g -DRT_ARM             \
        ${INC_PATH} ${SRC_LIST} -o simd_test.arm

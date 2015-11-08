
INC_PATH =                          \
        -I../core/

SRC_LIST =                          \
        simd_test.cpp

simd_test:
	g++ -O3 -g \
        -DRT_LINUX -DRT_ARM -DRT_128 -DRT_DEBUG=1 \
        ${INC_PATH} ${SRC_LIST} -o simd_test.arm

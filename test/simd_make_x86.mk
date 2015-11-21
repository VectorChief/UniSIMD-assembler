
INC_PATH =                          \
        -I../core/

SRC_LIST =                          \
        simd_test.cpp

simd_test:
	g++ -O3 -g -m32 \
        -DRT_LINUX -DRT_X86 -DRT_256=2 -DRT_DEBUG=1 \
        ${INC_PATH} ${SRC_LIST} -o simd_test.x86

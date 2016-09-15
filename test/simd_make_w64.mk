
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_w64_32 simd_test_w64f32 simd_test_w64f64

strip:
	strip simd_test_w64*.exe

clean:
	rm simd_test_w64*.exe


simd_test_w64_32:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test_w64_32.exe

simd_test_w64f32:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test_w64f32.exe

simd_test_w64f64:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test_w64f64.exe


# Prerequisites for the build:
# TDM64-GCC compiler for Win32/64 is installed and in the PATH variable.
# Download tdm64-gcc-5.1.0-2.exe from sourceforge and run the installer.
#
# Building/running SIMD test:
# run simd_make_w64.bat from Windows UI or
# run the following from Command Prompt "cmd":
# mingw32-make -f simd_make_w64.mk
# simd_test_w64f32.exe

# For SSE1 build use (replace): RT_128=1
# For SSE4 build use (replace): RT_128=4
# For AVX1 build use (replace): RT_256=1
# For AVX2 build use (replace): RT_256=2

# 64/32-bit (ptr/adr) hybrid mode compatible with native 64-bit ABI
# is available for the original pure 32-bit ISA using 64-bit pointers,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test_w64_32.exe

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# but currently requires addresses to be 64-bit as well (RT_ADDRESS=64),
# use (replace): RT_ELEMENT=64, rename the binary to simd_test_w64f64.exe

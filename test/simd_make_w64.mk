
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm

simd_test:
	g++ -O3 -g -static \
        -DRT_WIN64 -DRT_X64 -DRT_128=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test_w64f32.exe

# Prerequisites for the build:
# TDM64-GCC compiler for Win32/64 is installed and in the PATH variable.
# Download tdm64-gcc-5.1.0-2.exe from sourceforge and run the installer.
#
# Use "MinGW Command Prompt" from "Windows Start Menu" under "TDM-GCC-64".
# Works with regular "cmd" command prompt too after TDM64-GCC installation.
#
# Building/running SIMD test:
# run simd_make_w64.bat file or
# mingw32-make -f simd_make_w64.mk
# simd_test_w64f32.exe

# For SSE1 build use (replace): RT_128=1
# For SSE4 build use (replace): RT_128=4
# For AVX1 build use (replace): RT_256=1
# For AVX2 build use (replace): RT_256=2

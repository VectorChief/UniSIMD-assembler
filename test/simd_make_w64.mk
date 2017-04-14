
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_w64_32 simd_test_w64_64 simd_test_w64f32 simd_test_w64f64

strip:
	strip simd_test_w64*.exe

clean:
	del simd_test_w64*.exe


simd_test_w64_32:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_256_R8=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test_w64_32.exe

simd_test_w64_64:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_256_R8=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test_w64_64.exe

simd_test_w64f32:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_256_R8=2 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test_w64f32.exe

simd_test_w64f64:
	g++ -O3 -g -static -m64 \
        -DRT_WIN64 -DRT_X64 -DRT_256_R8=2 -DRT_DEBUG=0 \
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

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit SSE1 build use (replace): RT_128=1 (32-bit SIMD elements only)
# For 128-bit SSE2 build use (replace): RT_128=2
# For 128-bit SSE4 build use (replace): RT_128=4
# For 128-bit AVX1 build use (replace): RT_128=8
# For 128-bit AVX2 build use (replace): RT_128=8 RT_SIMD_COMPAT_128=2 (FMA3)

# For 256-bit SSE2 build use (replace): RT_256_R8=2       (8 SIMD-reg pairs)
# For 256-bit SSE4 build use (replace): RT_256_R8=4       (8 SIMD-reg pairs)
# For 256-bit AVX1 build use (replace): RT_256=1
# For 256-bit AVX2 build use (replace): RT_256=2

# For 512-bit AVX1 build use (replace): RT_512_R8=1       (8 SIMD-reg pairs)
# For 512-bit AVX2 build use (replace): RT_512_R8=2       (8 SIMD-reg pairs)
# For 512-bit AVX3.1 build use (replace): RT_512=1
# For 512-bit AVX3.2 build use (replace): RT_512=2

# For 1024-bit AVX3.1 build use (replace): RT_1K4=1
# For 1024-bit AVX3.2 build use (replace): RT_1K4=2
# For 2048-bit AVX3.1 build use (replace): RT_2K8_R8=1    (8 SIMD-reg quads)
# For 2048-bit AVX3.2 build use (replace): RT_2K8_R8=2    (8 SIMD-reg quads)

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test_w64_**.exe

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test_w64*64.exe

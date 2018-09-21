
INC_PATH =                              \
        -I../core/config/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: build_a64 build_sve

strip:
	aarch64-linux-gnu-strip simd_test.a64*

clean:
	rm simd_test.a64*


build_a64: simd_test_a64_32 simd_test_a64_64 simd_test_a64f32 simd_test_a64f64

simd_test_a64_32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64_32

simd_test_a64_64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64_64

simd_test_a64f32:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64f32

simd_test_a64f64:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_256=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64f64


build_sve: simd_test_a64_32sve simd_test_a64_64sve \
           simd_test_a64f32sve simd_test_a64f64sve

simd_test_a64_32sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_512=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64_32sve

simd_test_a64_64sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_512=4 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64_64sve

simd_test_a64f32sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_1K4=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64f32sve

simd_test_a64f64sve:
	aarch64-linux-gnu-g++ -O3 -g -static \
        -DRT_LINUX -DRT_A64 -DRT_1K4=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.a64f64sve


# On Ubuntu 16.04 Live DVD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for AArch64 is installed and in the PATH variable.
# sudo apt-get install g++-aarch64-linux-gnu
#
# Prerequisites for emulation:
# recent QEMU(-2.5) is installed or built from source and in the PATH variable.
# SVE target requires QEMU 3.0.0 with sve-max-vq cpu property patch.
# sudo apt-get install qemu
#
# Building/running SIMD test:
# make -f simd_make_a64.mk
# qemu-aarch64 -cpu cortex-a57 simd_test.a64_32 -c 1
# qemu-aarch64 -cpu cortex-a57 simd_test.a64_64 -c 1
# qemu-aarch64 -cpu cortex-a57 simd_test.a64f32 -c 1
# qemu-aarch64 -cpu cortex-a57 simd_test.a64f64 -c 1
# qemu-aarch64 -cpu max,sve-max-vq=2 simd_test.a64f32sve -c 1  (for RT_256=4)
# qemu-aarch64 -cpu max,sve-max-vq=2 simd_test.a64f64sve -c 1  (for RT_256=4)
# qemu-aarch64 -cpu max,sve-max-vq=2 simd_test.a64f32sve -c 1  (for RT_512=1)
# qemu-aarch64 -cpu max,sve-max-vq=2 simd_test.a64f64sve -c 1  (for RT_512=1)
# qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f32sve -c 1  (for RT_512=4)
# qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f64sve -c 1  (for RT_512=4)
# qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f32sve -c 1  (for RT_1K4=1)
# qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f64sve -c 1  (for RT_1K4=1)
# qemu-aarch64 -cpu max,sve-max-vq=8 simd_test.a64f32sve -c 1  (for RT_1K4=4)
# qemu-aarch64 -cpu max,sve-max-vq=8 simd_test.a64f64sve -c 1  (for RT_1K4=4)
# qemu-aarch64 -cpu max,sve-max-vq=8 simd_test.a64f32sve -c 1  (for RT_2K8_R8=1)
# qemu-aarch64 -cpu max,sve-max-vq=8 simd_test.a64f64sve -c 1  (for RT_2K8_R8=1)
# qemu-aarch64 -cpu max,sve-max-vq=16 simd_test.a64f32sve -c 1 (for RT_2K8_R8=4)
# qemu-aarch64 -cpu max,sve-max-vq=16 simd_test.a64f64sve -c 1 (for RT_2K8_R8=4)
# Use "-c 1" option to reduce test time when emulating with QEMU

# Clang compilation works too (takes much longer prior to 3.8), use (replace):
# clang++ (in place of ...-g++)
# sudo apt-get install clang

# For interpretation of SIMD build flags check compatibility layer in rtzero.h

# For 128-bit NEON build use (replace): RT_128=1            (30 SIMD registers)
# For 256-bit NEON build use (replace): RT_256=1            (15 SIMD reg-pairs)
# For 256-bit  SVEx1 build use (replace): RT_256=4          (30 SIMD registers)
# For 512-bit  SVEx2 build use (replace): RT_512=1          (15 SIMD registers)
# For 512-bit  SVEx1 build use (replace): RT_512=4          (30 SIMD registers)
# For 1024-bit SVEx2 build use (replace): RT_1K4=1          (15 SIMD registers)
# For 1024-bit SVEx1 build use (replace): RT_1K4=4          (30 SIMD registers)
# For 2048-bit SVEx2 build use (replace): RT_2K8_R8=1        (8 SIMD registers)
# For 2048-bit SVEx1 build use (replace): RT_2K8_R8=4       (15 SIMD registers)
# The last two slots are artificially reg-limited for compatibility with AVX512

# 64/32-bit (ptr/adr) hybrid mode is compatible with native 64-bit ABI,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.a64_**

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.a64*64

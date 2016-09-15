
INC_PATH =                              \
        -I../core/

SRC_LIST =                              \
        simd_test.cpp

LIB_PATH =

LIB_LIST =                              \
        -lm


build: simd_test_m64_32 simd_test_m64f32 simd_test_m64f64

strip:
	mips-img-linux-gnu-strip simd_test.m64*

clean:
	rm simd_test.m64*


simd_test_m64_32:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=32 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64_32

simd_test_m64f32:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=32 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f32

simd_test_m64f64:
	mips-img-linux-gnu-g++ -O3 -g -static -EL -mabi=64 -mmsa \
        -DRT_LINUX -DRT_M64=6 -DRT_128=1 -DRT_DEBUG=0 \
        -DRT_POINTER=64 -DRT_ADDRESS=64 -DRT_ELEMENT=64 -DRT_ENDIAN=0 \
        ${INC_PATH} ${SRC_LIST} ${LIB_PATH} ${LIB_LIST} -o simd_test.m64f64


# The up-to-date MIPS toolchain (g++ & QEMU) can be found here:
# https://community.imgtec.com/developers/mips/tools/codescape-mips-sdk/

# On Ubuntu 16.04 Live CD add "universe multiverse" to "main restricted"
# in /etc/apt/sources.list (sudo gedit /etc/apt/sources.list) then run:
# sudo apt-get update (ignoring the old database errors in the end)
#
# Prerequisites for the build:
# (cross-)compiler for MIPS+MSA is installed and in the PATH variable.
# Codescape.GNU.Tools.Package.2015.06-05.for.MIPS.IMG.Linux.CentOS-5.x86_64
# is unpacked and folder mips-img-linux-gnu/2015.06-05/bin is added to PATH:
# PATH=/home/ubuntu/Downloads/mips-img-linux-gnu/2015.06-05/bin:$PATH
#
# Prerequisites for emulation:
# QEMU 2.4.0.1.0 from imgtec.com is built from source and in the PATH variable.
# Unpack qemu-rel-2.4.0.1.0 archive and change to its root folder, then run:
# sudo apt-get install zlib1g-dev libglib2.0-dev libpixman-1-dev
# ./configure --target-list=mips64el-linux-user,mips64-linux-user
# make -j8
# sudo make install
# (building from source makes QEMU's mmap forwarding honour address hint)
#
# Building/running SIMD test:
# make -f simd_make_m64.mk
# qemu-mips64el -cpu I6400 simd_test.m64f32

# For big-endian MIPS64 target use (replace): -EB -DRT_ENDIAN=1
# qemu-mips64 -cpu I6400 simd_test.m64f32

# 64/32-bit (ptr/adr) hybrid mode compatible with native 64-bit ABI
# is available for the original pure 32-bit ISA using 64-bit pointers,
# use (replace): RT_ADDRESS=32, rename the binary to simd_test.m64_32

# 64-bit packed SIMD mode (fp64/int64) is supported on 64-bit targets,
# but currently requires addresses to be 64-bit as well (RT_ADDRESS=64),
# use (replace): RT_ELEMENT=64, rename the binary to simd_test.m64f64

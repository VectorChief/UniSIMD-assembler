#!/bin/sh
# Intended for x86_64 Linux build environment
# with many g++ cross-compilers installed (64-bit Ubuntu 16.04 LTS tested)

make -f simd_make_arm.mk build -j2
make -f simd_make_m32.mk build -j4
make -f simd_make_p32.mk build -j2
make -f simd_make_a64.mk build -j4
make -f simd_make_m64.mk build -j8
make -f simd_make_p64.mk build -j8

make -f simd_make_arm.mk strip
make -f simd_make_m32.mk strip
make -f simd_make_p32.mk strip
make -f simd_make_a64.mk strip
make -f simd_make_m64.mk strip
make -f simd_make_p64.mk strip

#!/bin/sh
# Intended for x86_64 Linux build environment
# with many g++ cross-compilers installed (64-bit Ubuntu MATE 20.04 LTS tested)
# refer to individual makefiles for installation instructions

make -f simd_make_arm.mk clean
make -f simd_make_m32.mk clean
make -f simd_make_p32.mk clean
make -f simd_make_a64.mk clean
make -f simd_make_m64.mk clean
make -f simd_make_p64.mk clean

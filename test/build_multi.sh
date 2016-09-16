#!/bin/sh
# Intended for x86_64 Linux build environment
# with native g++ multilib-compiler installed (64-bit Linux Mint 17 tested)

make -f simd_make_x86.mk build
make -f simd_make_x32.mk build
make -f simd_make_x64.mk build

make -f simd_make_x86.mk strip
make -f simd_make_x32.mk strip
make -f simd_make_x64.mk strip

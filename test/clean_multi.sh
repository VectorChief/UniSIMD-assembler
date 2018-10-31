#!/bin/sh
# Intended for x86_64 Linux build environment
# with native g++ multilib-compiler installed (64-bit Linux Mint 18 tested)

make -f simd_make_x86.mk clean
make -f simd_make_x32.mk clean

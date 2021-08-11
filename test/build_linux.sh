#!/bin/sh
# Intended for x86_64 Linux build environment
# with native g++ compiler installed (64-bit Linux Mint 18 tested)
# works on Ubuntu MATE 18.04/20.04 LTS (binaries aren't backward compatible)

make -f simd_make_x64.mk build -j8

make -f simd_make_x64.mk strip

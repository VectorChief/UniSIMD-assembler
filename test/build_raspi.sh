#!/bin/sh
# Intended for ARMv7 Linux build environment
# with native g++ compiler installed (32-bit Raspbian 7 and 8 tested)

make -f simd_make_arm.mk build_rpiX -j2

make -f simd_make_arm.mk strip_rpiX

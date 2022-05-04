#!/bin/sh
# Intended for AArch64 macOS build environment with Apple Silicon (M1 chip)
# with Command Line Tools installed (macOS BigSur/Monterey tested)
# build on the least recent OS as binaries aren't always backward compatible

make -f simd_make_a64.mk macRM

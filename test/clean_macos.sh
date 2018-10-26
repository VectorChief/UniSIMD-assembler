#!/bin/sh
# Intended for x86_64 Mac OS X / OS X / macOS build environment
# with Command Line Tools installed (Mac OS X Lion / macOS High Sierra tested)

make -f simd_make_x86.mk macRM
make -f simd_make_x64.mk macRM

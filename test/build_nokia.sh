#!/bin/sh
# Intended for scratchbox Linux build environment (32-bit Ubuntu 10.10 tested)
# http://wiki.maemo.org/Documentation/Maemo_5_Final_SDK_Installation

make -f simd_make_arm.mk build_n900

make -f simd_make_arm.mk strip_n900

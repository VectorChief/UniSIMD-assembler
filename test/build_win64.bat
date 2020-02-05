:: Intended for x86_64 Windows build environment
:: with TDM64-GCC compiler installed (64-bit Windows 7 SP1, Windows 10 tested)

mingw32-make -f simd_make_w64.mk build -j4

mingw32-make -f simd_make_w64.mk strip

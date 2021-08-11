#!/bin/sh
# Intended for x86_64 Linux test environment
# with multilib capabilities (64-bit Linux Mint 18 tested)
# run this script after bulid_multi.sh with multilib-compiler installed

touch test86; rm test86

# fully successful test pass results in test86 file of  30356 bytes (51 tests)
# test pass on AVX2-only CPU results in test86 file of  23474 bytes (51 tests)
# for any other CPU check the output or use Intel SDE within script


echo "========================================================" | tee -a test86
echo "Testing x86 target (Intel Core 2 Duo SSE2)" | tee -a test86
echo "========================================================" | tee -a test86
./simd_test.x86 -c 1 | tee -a test86
echo "========================================================" | tee -a test86
echo "Testing x86avx target (Intel Sandy Bridge AVX1)" | tee -a test86
echo "========================================================" | tee -a test86
./simd_test.x86avx -c 1 | tee -a test86
echo "========================================================" | tee -a test86
echo "Testing x86avx512 target (Intel Xeon Phi KNL AVX512)" | tee -a test86
echo "========================================================" | tee -a test86
./simd_test.x86avx512 -c 1 | tee -a test86
echo "========================================================" | tee -a test86
echo "Testing x32 target (Intel Core 2 Duo SSE2)" | tee -a test86
echo "========================================================" | tee -a test86
./simd_test.x32 -c 1 | tee -a test86


echo "========================================================"
echo "fully successful test pass writes  30356 bytes to test86"
echo "test pass on AVX2-only CPU writes  23474 bytes to test86"
echo "for other CPUs check the output, use Intel SDE in script"
echo "========================================================"
echo "the actual file size after the test run is listed below:"
ls -al test86
echo "========================================================"



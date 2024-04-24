#!/bin/sh
# Intended for x86_64 Linux test environment
# tested on 64-bit Linux Mint 18, 64-bit Ubuntu MATE 18.04/20.04 LTS
# run this script after bulid_linux.sh with native compiler installed

touch test64; rm test64

# fully successful test pass results in test64 file of  99666 bytes (51 tests)
# test pass on AVX2-only CPU results in test64 file of  69286 bytes (51 tests)
# for any other CPU check the output or use Intel SDE within script


echo "========================================================" | tee -a test64
echo "Testing x64_32 target (Intel Core 2 Duo SSE2)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64_32 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64_64 target (Intel Core 2 Duo SSE2)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64_64 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64f32 target (Intel Nehalem SSE4)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64f32 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64f64 target (Intel Nehalem SSE4)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64f64 -c 1 | tee -a test64

echo "========================================================" | tee -a test64
echo "Testing x64_32avx target (Intel Sandy Bridge AVX1)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64_32avx -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64_64avx target (Intel Sandy Bridge AVX1)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64_64avx -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64f32avx target (Intel Haswell AVX2)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64f32avx -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64f64avx target (Intel Haswell AVX2)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64f64avx -c 1 | tee -a test64

echo "========================================================" | tee -a test64
echo "Testing x64_32avx512 target (Intel Xeon Phi KNL AVX512)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64_32avx512 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64_64avx512 target (Intel Xeon Phi KNL AVX512)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64_64avx512 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64f32avx512 target (Intel Rocket Lake AVX512)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64f32avx512 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing x64f64avx512 target (Intel Rocket Lake AVX512)" | tee -a test64
echo "========================================================" | tee -a test64
./simd_test.x64f64avx512 -c 1 | tee -a test64


echo "========================================================"
echo "fully successful test pass writes  99666 bytes to test64"
echo "test pass on AVX2-only CPU writes  69286 bytes to test64"
echo "for other CPUs check the output, use Intel SDE in script"
echo "========================================================"
echo "the actual file size after the test run is listed below:"
ls -al test64
echo "========================================================"



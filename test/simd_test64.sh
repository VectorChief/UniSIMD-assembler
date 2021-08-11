#!/bin/sh
# Intended for x86_64 Linux test environment
# with QEMU linux-user mode installed (64-bit Ubuntu MATE 20.04 LTS tested)

touch test64; rm test64

# fully successful test pass results in test64 file of 183950 bytes (29 tests)
# test pass on AVX2-only CPU results in test64 file of 168394 bytes (29 tests)
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


echo "========================================================" | tee -a test64
echo "Testing a64_32 target (ARMv8 NEON)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu cortex-a57 simd_test.a64_32 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing a64_64 target (ARMv8 NEON)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu cortex-a57 simd_test.a64_64 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing a64f32 target (ARMv8 NEON)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu cortex-a57 simd_test.a64f32 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing a64f64 target (ARMv8 NEON)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu cortex-a57 simd_test.a64f64 -c 1 | tee -a test64

echo "========================================================" | tee -a test64
echo "Testing a64_32sve target (ARMv8 SVE)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64_32sve -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing a64_64sve target (ARMv8 SVE)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64_64sve -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing a64f32sve target (ARMv8 SVE)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f32sve -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing a64f64sve target (ARMv8 SVE)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f64sve -c 1 | tee -a test64


echo "========================================================" | tee -a test64
echo "Testing m64_32Lr6 target (MIPS64r6 MSA little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64el -cpu I6400 simd_test.m64_32Lr6 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing m64_64Lr6 target (MIPS64r6 MSA little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64el -cpu I6400 simd_test.m64_64Lr6 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing m64f32Lr6 target (MIPS64r6 MSA little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64el -cpu I6400 simd_test.m64f32Lr6 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing m64f64Lr6 target (MIPS64r6 MSA little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64el -cpu I6400 simd_test.m64f64Lr6 -c 1 | tee -a test64

echo "========================================================" | tee -a test64
echo "Testing m64_32Br6 target (MIPS64r6 MSA    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64   -cpu I6400 simd_test.m64_32Br6 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing m64_64Br6 target (MIPS64r6 MSA    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64   -cpu I6400 simd_test.m64_64Br6 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing m64f32Br6 target (MIPS64r6 MSA    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64   -cpu I6400 simd_test.m64f32Br6 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing m64f64Br6 target (MIPS64r6 MSA    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-mips64   -cpu I6400 simd_test.m64f64Br6 -c 1 | tee -a test64


echo "========================================================" | tee -a test64
echo "Testing p64_32Bp7 target (POWER7 VSX1    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64   -cpu POWER7 simd_test.p64_32Bp7 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64_64Bp7 target (POWER7 VSX1    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64   -cpu POWER7 simd_test.p64_64Bp7 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64f32Bp7 target (POWER7 VSX1    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64   -cpu POWER7 simd_test.p64f32Bp7 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64f64Bp7 target (POWER7 VSX1    big-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64   -cpu POWER7 simd_test.p64f64Bp7 -c 1 | tee -a test64

echo "========================================================" | tee -a test64
echo "Testing p64_32Lp8 target (POWER8 VSX2 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER8 simd_test.p64_32Lp8 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64_64Lp8 target (POWER8 VSX2 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER8 simd_test.p64_64Lp8 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64f32Lp8 target (POWER8 VSX2 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER8 simd_test.p64f32Lp8 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64f64Lp8 target (POWER8 VSX2 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER8 simd_test.p64f64Lp8 -c 1 | tee -a test64

echo "========================================================" | tee -a test64
echo "Testing p64_32Lp9 target (POWER9 VSX3 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER9 simd_test.p64_32Lp9 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64_64Lp9 target (POWER9 VSX3 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER9 simd_test.p64_64Lp9 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64f32Lp9 target (POWER9 VSX3 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER9 simd_test.p64f32Lp9 -c 1 | tee -a test64
echo "========================================================" | tee -a test64
echo "Testing p64f64Lp9 target (POWER9 VSX3 little-endian)" | tee -a test64
echo "========================================================" | tee -a test64
qemu-ppc64le -cpu POWER9 simd_test.p64f64Lp9 -c 1 | tee -a test64


echo "========================================================"
echo "fully successful test pass writes 183950 bytes to test64"
echo "test pass on AVX2-only CPU writes 168394 bytes to test64"
echo "for other CPUs check the output, use Intel SDE in script"
echo "========================================================"
echo "the actual file size after the test run is listed below:"
ls -al test64
echo "========================================================"



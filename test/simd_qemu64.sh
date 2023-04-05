#!/bin/sh
# Intended for x86_64 Linux test environment
# with QEMU linux-user mode installed (64-bit Ubuntu MATE 20.04 LTS tested)
# run this script after bulid_cross.sh with 64-bit cross-compilers installed

touch qemu64; rm qemu64

# fully successful test pass results in qemu64 file of 155412 bytes (36 tests)
# unlike simd_test64/86.sh the result is the same on all CPU types  (36 tests)
# check the output if qemu64 file size differs, look for printouts


echo "========================================================" | tee -a qemu64
echo "Testing a64_32 target (ARMv8 NEON)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu cortex-a57 simd_test.a64_32 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing a64_64 target (ARMv8 NEON)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu cortex-a57 simd_test.a64_64 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing a64f32 target (ARMv8 NEON)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu cortex-a57 simd_test.a64f32 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing a64f64 target (ARMv8 NEON)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu cortex-a57 simd_test.a64f64 -c 1 | tee -a qemu64

echo "========================================================" | tee -a qemu64
echo "Testing a64_32sve target (ARMv8 SVE)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64_32sve -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing a64_64sve target (ARMv8 SVE)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64_64sve -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing a64f32sve target (ARMv8 SVE)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f32sve -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing a64f64sve target (ARMv8 SVE)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-aarch64 -cpu max,sve-max-vq=4 simd_test.a64f64sve -c 1 | tee -a qemu64


echo "========================================================" | tee -a qemu64
echo "Testing m64_32Lr6 target (MIPS64r6 MSA little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64el -cpu I6400 simd_test.m64_32Lr6 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing m64_64Lr6 target (MIPS64r6 MSA little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64el -cpu I6400 simd_test.m64_64Lr6 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing m64f32Lr6 target (MIPS64r6 MSA little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64el -cpu I6400 simd_test.m64f32Lr6 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing m64f64Lr6 target (MIPS64r6 MSA little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64el -cpu I6400 simd_test.m64f64Lr6 -c 1 | tee -a qemu64

echo "========================================================" | tee -a qemu64
echo "Testing m64_32Br6 target (MIPS64r6 MSA    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64   -cpu I6400 simd_test.m64_32Br6 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing m64_64Br6 target (MIPS64r6 MSA    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64   -cpu I6400 simd_test.m64_64Br6 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing m64f32Br6 target (MIPS64r6 MSA    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64   -cpu I6400 simd_test.m64f32Br6 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing m64f64Br6 target (MIPS64r6 MSA    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-mips64   -cpu I6400 simd_test.m64f64Br6 -c 1 | tee -a qemu64


echo "========================================================" | tee -a qemu64
echo "Testing p64_32Bp7 target (POWER7 VSX1    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64   -cpu POWER7 simd_test.p64_32Bp7 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64_64Bp7 target (POWER7 VSX1    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64   -cpu POWER7 simd_test.p64_64Bp7 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64f32Bp7 target (POWER7 VSX1    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64   -cpu POWER7 simd_test.p64f32Bp7 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64f64Bp7 target (POWER7 VSX1    big-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64   -cpu POWER7 simd_test.p64f64Bp7 -c 1 | tee -a qemu64

# forcing -cpu power9 for power8 targets is a workaround for Ubuntu 22.04 LTS
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109007

echo "========================================================" | tee -a qemu64
echo "Testing p64_32Lp8 target (POWER8 VSX2 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64_32Lp8 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64_64Lp8 target (POWER8 VSX2 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64_64Lp8 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64f32Lp8 target (POWER8 VSX2 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64f32Lp8 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64f64Lp8 target (POWER8 VSX2 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64f64Lp8 -c 1 | tee -a qemu64

echo "========================================================" | tee -a qemu64
echo "Testing p64_32Lp9 target (POWER9 VSX3 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64_32Lp9 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64_64Lp9 target (POWER9 VSX3 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64_64Lp9 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64f32Lp9 target (POWER9 VSX3 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64f32Lp9 -c 1 | tee -a qemu64
echo "========================================================" | tee -a qemu64
echo "Testing p64f64Lp9 target (POWER9 VSX3 little-endian)" | tee -a qemu64
echo "========================================================" | tee -a qemu64
qemu-ppc64le -cpu POWER9 simd_test.p64f64Lp9 -c 1 | tee -a qemu64


echo "========================================================"
echo "fully successful test pass writes 155412 bytes to qemu64"
echo "the result doesn't depend on CPU type (unlike test64/86)"
echo "check the output if qemu64 size differs, check printouts"
echo "========================================================"
echo "the actual file size after the test run is listed below:"
ls -al qemu64
echo "========================================================"



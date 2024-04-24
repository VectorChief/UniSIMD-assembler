#!/bin/sh
# Intended for x86_64 Linux test environment
# with QEMU linux-user mode installed (64-bit Ubuntu MATE 20.04 LTS tested)
# run this script after bulid_cross.sh with 32-bit cross-compilers installed

touch qemu32; rm qemu32

# fully successful test pass results in qemu32 file of  41524 bytes (51 tests)
# unlike simd_test64/86.sh the result is the same on all CPU types  (51 tests)
# check the output if qemu32 file size differs, look for printouts


echo "========================================================" | tee -a qemu32
echo "Testing arm_v1 target (ARMv7 Cortex-A8  NEON)" | tee -a qemu32
echo "========================================================" | tee -a qemu32
qemu-arm -cpu cortex-a8  simd_test.arm_v1 -c 1 | tee -a qemu32
echo "========================================================" | tee -a qemu32
echo "Testing arm_v2 target (ARMv7 Cortex-A15 NEON)" | tee -a qemu32
echo "========================================================" | tee -a qemu32
qemu-arm -cpu cortex-a15 simd_test.arm_v2 -c 1 | tee -a qemu32


echo "========================================================" | tee -a qemu32
echo "Testing m32Lr5 target (MIPS32r5 MSA little-endian)" | tee -a qemu32
echo "========================================================" | tee -a qemu32
qemu-mipsel -cpu P5600 simd_test.m32Lr5 -c 1 | tee -a qemu32
echo "========================================================" | tee -a qemu32
echo "Testing m32Br5 target (MIPS32r5 MSA    big-endian)" | tee -a qemu32
echo "========================================================" | tee -a qemu32
qemu-mips   -cpu P5600 simd_test.m32Br5 -c 1 | tee -a qemu32


# ppc64abi32 targets are deprecated since QEMU 5.2.0 (dropped in Ubuntu 22.04)
# fully successful test pass writes 66442 bytes to qemu32 with ppc64abi32 runs

echo "========================================================" | tee -a qemu32
echo "Testing p32Bg4 target (PPC G4 VMX     big-endian)" | tee -a qemu32
echo "========================================================" | tee -a qemu32
qemu-ppc        -cpu G4     simd_test.p32Bg4 -c 1 | tee -a qemu32
#echo "========================================================" | tee -a qemu32
#echo "Testing p32Bp7 target (POWER7 VSX1    big-endian)" | tee -a qemu32
#echo "========================================================" | tee -a qemu32
#qemu-ppc64abi32 -cpu POWER7 simd_test.p32Bp7 -c 1 | tee -a qemu32
#echo "========================================================" | tee -a qemu32
#echo "Testing p32Bp8 target (POWER8 VSX2    big-endian)" | tee -a qemu32
#echo "========================================================" | tee -a qemu32
#qemu-ppc64abi32 -cpu POWER8 simd_test.p32Bp8 -c 1 | tee -a qemu32
#echo "========================================================" | tee -a qemu32
#echo "Testing p32Bp9 target (POWER9 VSX3    big-endian)" | tee -a qemu32
#echo "========================================================" | tee -a qemu32
#qemu-ppc64abi32 -cpu POWER9 simd_test.p32Bp9 -c 1 | tee -a qemu32


echo "========================================================"
echo "fully successful test pass writes  41524 bytes to qemu32"
echo "the result doesn't depend on CPU type (unlike test64/86)"
echo "check the output if qemu32 size differs, check printouts"
echo "========================================================"
echo "the actual file size after the test run is listed below:"
ls -al qemu32
echo "========================================================"



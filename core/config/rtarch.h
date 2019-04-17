/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_H
#define RT_RTARCH_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch.h: Architecture selection file (apps include rtbase.h as root).
 * The initial documentation is provided below with links to other files.
 *
 * Unified SIMD assembler framework consists of multiple header files and is
 * designed to be compatible with different processor architectures,
 * while maintaining clearly defined common API. Namespaces for current
 * and future instruction subsets are defined (reserved) in the next section.
 *
 * Definitions provided in this file are intended to hide the differences of
 * inline assembly implementations in various compilers and operating systems,
 * while definitions of particular instruction sets are delegated to the
 * corresponding companion files named rtarch_***.h for BASE instructions
 * and rtarch_***_***x*v*.h for SIMD instructions.
 *
 * Note that AArch32 mode of ARMv8 ISA is a part of legacy ARM target as it
 * only brings SIMD fp-convert with round parameter and other minor tweaks,
 * while IEEE-compatible SIMD fp-arithmetic with full square root and divide
 * is exposed in AArch64 mode of ARMv8 ISA via A32 and A64 targets (fp32+fp64).
 *
 * Current naming scheme for legacy, modern and potential future targets.
 *
 * Legacy 32/64-bit BASE
 *   plus 32/64-bit SIMD combinations:
 *
 *  - rtarch_arm.h         - AArch32:ARMv7 ISA, 16 BASE regs, 8+temps used
 *  - rtarch_arm_128x1v4.h - fp32, 16 SIMD regs, NEON 128-bit, 8+temps used
 *  - rtarch_p32.h         - POWER 32-bit ISA, 32 BASE regs, 14+temps used
 *  - rtarch_p32_128x1v4.h - fp32, 32 SIMD regs, VMX 128-bit, 15+temps used
 *  - rtarch_p32_128x2vG.h - fp32, 32 SIMD regs, 8 128-bit pairs, 15+temps used
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 BASE regs, 14+temps used
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 BASE regs, 14+temps used
 *  - rtarch_x32_128x1v4.h - fp32, 16 SIMD regs, SSE 128-bit, all 16 can be used
 *  - rtarch_x64_128x1v4.h - fp64, 16 SIMD regs, SSE 128-bit, all 16 can be used
 *  - rtarch_x32_128x2v4.h - fp32, 16 SIMD regs, 8 SSE 128-bit pairs, 16 used
 *  - rtarch_x64_128x2v4.h - fp64, 16 SIMD regs, 8 SSE 128-bit pairs, 16 used
 *  - rtarch_x32_256x2v2.h - fp32, 16 SIMD regs, 8 AVX 256-bit pairs, 16 used
 *  - rtarch_x64_256x2v2.h - fp64, 16 SIMD regs, 8 AVX 256-bit pairs, 16 used
 *  - rtarch_x32_512x4v2.h - fp32, 32 SIMD regs, 8 AVX 512-bit quads, 32 used
 *  - rtarch_x64_512x4v2.h - fp64, 32 SIMD regs, 8 AVX 512-bit quads, 32 used
 *  - rtarch_x86.h         - x86 32-bit ISA, 8 BASE regs, 6+esp/ebp used
 *  - rtarch_x86_128x1v4.h - fp32, 8 SIMD regs, SSE 128-bit, all 8 used
 *  - rtarch_x86_256x1v2.h - fp32, 8 SIMD regs, AVX 256-bit, all 8 used
 *  - rtarch_x86_512x1v2.h - fp32, 8 SIMD regs, AVX 512-bit, all 8 used
 *
 * Modern 32/64-bit BASE
 *   plus 32/64-bit SIMD combinations:
 *
 *  - rtarch_a32.h         - AArch64:ILP32 ABI, 32 BASE regs, 14+temps used
 *  - rtarch_a64.h         - AArch64:ARMv8 ISA, 32 BASE regs, 14+temps used
 *  - rtarch_a32_128x1v1.h - fp32, 32 SIMD regs, AdvSIMD 128-bit, 30+temps used
 *  - rtarch_a64_128x1v1.h - fp64, 32 SIMD regs, AdvSIMD 128-bit, 30+temps used
 *  - rtarch_a32_128x2v1.h - fp32, 32 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_a64_128x2v1.h - fp64, 32 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_a32_SVEx1v1.h - fp32, 32 SIMD regs, SVE 256:2048-bit, 30+temps used
 *  - rtarch_a64_SVEx1v1.h - fp64, 32 SIMD regs, SVE 256:2048-bit, 30+temps used
 *  - rtarch_a32_SVEx2v1.h - fp32, 32 SIMD regs, 15 ARM-SVE pairs, 30+temps used
 *  - rtarch_a64_SVEx2v1.h - fp64, 32 SIMD regs, 15 ARM-SVE pairs, 30+temps used
 *  - rtarch_m32.h         - MIPS32 r5/r6 ISA, 32 BASE regs, 14+temps used
 *  - rtarch_m64.h         - MIPS64 r5/r6 ISA, 32 BASE regs, 14+temps used
 *  - rtarch_m32_128x1v1.h - fp32, 32 SIMD regs, MSA 128-bit, 30+temps used
 *  - rtarch_m64_128x1v1.h - fp64, 32 SIMD regs, MSA 128-bit, 30+temps used
 *  - rtarch_m32_128x2v1.h - fp32, 32 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_m64_128x2v1.h - fp64, 32 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_p32.h         - POWER 32-bit ISA, 32 BASE regs, 14+temps used
 *  - rtarch_p64.h         - POWER 64-bit ISA, 32 BASE regs, 14+temps used
 *  - rtarch_p32_128x1v1.h - fp32, 64 SIMD regs, VSX1+2 128-bit, 30+temps used
 *  - rtarch_p64_128x1v1.h - fp64, 64 SIMD regs, VSX1+2 128-bit, 30+temps used
 *  - rtarch_p32_128x2v1.h - fp32, 64 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_p64_128x2v1.h - fp64, 64 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_p32_128x2v4.h - fp32, 64 SIMD regs, 30 128-bit pairs, 60+temps used
 *  - rtarch_p64_128x2v4.h - fp64, 64 SIMD regs, 30 128-bit pairs, 60+temps used
 *  - rtarch_p32_128x4v1.h - fp32, 64 SIMD regs, 15 128-bit quads, 60+temps used
 *  - rtarch_p64_128x4v1.h - fp64, 64 SIMD regs, 15 128-bit quads, 60+temps used
 *  - rtarch_p32_128x1v2.h - fp32, 64 SIMD regs, VSX3 128-bit, 30+temps used
 *  - rtarch_p64_128x1v2.h - fp64, 64 SIMD regs, VSX3 128-bit, 30+temps used
 *  - rtarch_p32_128x2v2.h - fp32, 64 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_p64_128x2v2.h - fp64, 64 SIMD regs, 15 128-bit pairs, 30+temps used
 *  - rtarch_p32_128x2v8.h - fp32, 64 SIMD regs, 30 128-bit pairs, 60+temps used
 *  - rtarch_p64_128x2v8.h - fp64, 64 SIMD regs, 30 128-bit pairs, 60+temps used
 *  - rtarch_p32_128x4v2.h - fp32, 64 SIMD regs, 15 128-bit quads, 60+temps used
 *  - rtarch_p64_128x4v2.h - fp64, 64 SIMD regs, 15 128-bit quads, 60+temps used
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 BASE regs, 14+temps used
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 BASE regs, 14+temps used
 *  - rtarch_x32_128x1v2.h - fp32, 32 SIMD regs, AVX-512+ 128-bit, only 30 used
 *  - rtarch_x64_128x1v2.h - fp64, 32 SIMD regs, AVX-512+ 128-bit, only 30 used
 *  - rtarch_x32_128x1v8.h - fp32, 16 SIMD regs, AVX 128-bit, all 16 can be used
 *  - rtarch_x64_128x1v8.h - fp64, 16 SIMD regs, AVX 128-bit, all 16 can be used
 *  - rtarch_x32_256x1v2.h - fp32, 16 SIMD regs, AVX 256-bit, all 16 can be used
 *  - rtarch_x64_256x1v2.h - fp64, 16 SIMD regs, AVX 256-bit, all 16 can be used
 *  - rtarch_x32_256x1v8.h - fp32, 32 SIMD regs, AVX-512+ 256-bit, only 30 used
 *  - rtarch_x64_256x1v8.h - fp64, 32 SIMD regs, AVX-512+ 256-bit, only 30 used
 *  - rtarch_x32_512x1v8.h - fp32, 32 SIMD regs, AVX-512+ 512-bit, only 30 used
 *  - rtarch_x64_512x1v8.h - fp64, 32 SIMD regs, AVX-512+ 512-bit, only 30 used
 *  - rtarch_x32_512x2v2.h - fp32, 32 SIMD regs, 16 512-bit pairs, 32 used
 *  - rtarch_x64_512x2v2.h - fp64, 32 SIMD regs, 16 512-bit pairs, 32 used
 *
 * Future 32/64-bit BASE
 *   plus 32/64-bit SIMD combinations:
 *
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 BASE regs, 14+temps used
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 BASE regs, 14+temps used
 *  - rtarch_x32_128x1v1.h - fp32, 16 SIMD regs, SSE 128-bit, 30 regs emulated
 *  - rtarch_x64_128x1v1.h - fp64, 16 SIMD regs, SSE 128-bit, 30 regs emulated
 *  - rtarch_x32_256x1v4.h - fp32, 16 SIMD regs, AVX 256-bit, 30 regs emulated
 *  - rtarch_x64_256x1v4.h - fp64, 16 SIMD regs, AVX 256-bit, 30 regs emulated
 *  - rtarch_r32.h         - RISC-V 32-bit ISA, 32 BASE regs, 14+temps used
 *  - rtarch_r64.h         - RISC-V 64-bit ISA, 32 BASE regs, 14+temps used
 *  - rtarch_r32_***x*v*.h - fp32, SIMD alternative for RISC-V
 *  - rtarch_r64_***x*v*.h - fp64, SIMD alternative for RISC-V
 *
 * not all registers in target descriptions are always exposed for apps to use
 * flags RT_BASE_REGS and RT_SIMD_REGS are available for rough differentiation
 * between register-file sizes with current values: legacy 8, 16 and 32 (SIMD)
 * while top registers reservation is controlled via RT_SIMD_COMPAT_XMM option
 *
 * Current naming scheme for extended BASE and SIMD register-files.
 *
 * Legacy 8 BASE and 8 SIMD registers:
 *  - Reax, Rebx, Recx, Redx, Resp, Rebp, Resi, Redi
 *  - Xmm0, Xmm1, Xmm2, Xmm3, Xmm4, Xmm5, Xmm6, Xmm7
 *
 * Modern 16 BASE and 16 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegF
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmF
 *
 * Modern 16 BASE and 32 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegF
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmV
 *
 * Although register names are fixed, register sizes are not and depend
 * on the chosen target (32/64-bit BASE and 128/256/512/1K4/2K8-bit SIMD).
 * Base registers can be 32/64-bit wide, while their SIMD counterparts
 * depend on the architecture and SIMD version chosen for the target.
 * On 64-bit systems SIMD can be configured to work with 32/64-bit elements.
 * Fractional sub-registers don't have names and aren't architecturally
 * visible in the assembler in order to simplify SPMD programming model.
 */

/******************************************************************************/

/*
 *** The following BASE instruction namespaces are defined for current use.
 *
 * cmdw*_** - word-size args, BASE ISA (data element is always fixed at 32-bit)
 * cmdx*_** - addr-size args, BASE ISA (32/64-bit configurable with RT_ADDRESS)
 * cmdy*_** - elem-size args, BASE ISA (32/64-bit configurable with RT_ELEMENT)
 * cmdz*_** - full-size args, BASE ISA (data element is always fixed at 64-bit)
 *
 * cmd*z_** - usage for setting-flags is implemented orthogonal to data-size
 *
 *** The following SIMD instruction namespaces are defined for current use.
 *
 * cmdr*_** - SIMD-elem args, ELEM ISA (data element is 32-bit, scalar)
 * cmds*_** - SIMD-elem args, ELEM ISA (32/64-bit configurable, scalar)
 * cmdt*_** - SIMD-elem args, ELEM ISA (data element is 64-bit, scalar)
 *
 * cmde*_** - extd-size args, extd ISA (for 80-bit extended double, x87)
 *
 *** The following SIMD instruction namespaces are defined for current use.
 *
 * cmdo*_** - SIMD-data args, SIMD ISA (data element is 32-bit, packed-var-len)
 * cmdp*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-var-len)
 * cmdq*_** - SIMD-data args, SIMD ISA (data element is 64-bit, packed-var-len)
 *
 * packed SIMD instructions above are vector-length-agnostic: 128-bit multiples
 *
 * cmdi*_** - SIMD-data args, SIMD ISA (data element is 32-bit, packed-128-bit)
 * cmdj*_** - SIMD-data args, SIMD ISA (data element is 64-bit, packed-128-bit)
 * cmdl*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-128-bit)
 *
 * cmdc*_** - SIMD-data args, SIMD ISA (data element is 32-bit, packed-256-bit)
 * cmdd*_** - SIMD-data args, SIMD ISA (data element is 64-bit, packed-256-bit)
 * cmdf*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-256-bit)
 *
 * fixed 256-bit ops can be done as pairs with 2*15 128-bit regs on modern RISCs
 * fixed 256-bit ops can be done as pairs with 2*30 128-bit regs on modern POWER
 * while 512-bit ops can be done as quads with 4*15 128-bit regs on modern POWER
 * potentially saving extra address-calculation step using indexed-load on POWER
 * in case of only 14/28 exposed SIMD registers some rare ops can be done faster
 *
 * cvx*s_** - SIMD-data args, SIMD ISA (convert to fp-precision below selected)
 * cvy*s_** - SIMD-data args, SIMD ISA (convert to fp-precision above selected)
 * conversion to lower fp-precision narrows onto lower-half of selected vec-size
 * conversion to higher fp-precision widens from lower-half of selected vec-size
 * cux*s_**, cuy*s_** are reserved for fp-precision conversion of the upper-half
 *
 * cu**s_rr, cu**s_ld are reserved for fp-to-unsigned-int conversion, keeps size
 * cv**n_rr, cv**n_ld already in use for signed-int-to-fp conversion, keeps size
 * cv**x_rr, cv**x_ld are reserved for unsigned-int-to-fp conversion, keeps size
 * cv***F**, c***sF** can reuse fp<->int names for .?-sized fixed-point, ? = F*8
 * when fp<->int sizes don't match extended fpu ISA can be used with other archs
 *
 *** The following BASE instruction namespaces are reserved for the future.
 *
 * cmdb*_** - byte-size args, BASE ISA (displacement/alignment may differ)
 * cmdh*_** - half-size args, BASE ISA (displacement/alignment may differ)
 *
 * cmdk*_** - king-kong args, BASE ISA (for 128-bit BASE subset, RISC-V)
 *
 *** The following SIMD instruction namespaces are reserved for the future.
 *
 * cmdm*_** - SIMD-data args, SIMD ISA (packed fp16/int subset, half-precision)
 * cmdn*_** - SIMD-elem args, ELEM ISA (scalar fp16/int subset, half-precision)
 *
 * cmdu*_** - SIMD-data args, SIMD ISA (packed f128/int subset, quad-precision)
 * cmdv*_** - SIMD-elem args, ELEM ISA (scalar f128/int subset, quad-precision)
 *
 * cmdpb_** - SIMD-data args, uint ISA (packed byte-int subset)
 * cmdph_** - SIMD-data args, uint ISA (packed half-int subset)
 *
 * packed SIMD instructions above are vector-length-agnostic: 128-bit multiples
 *
 * cmdg*_** - SIMD-data args, SIMD ISA (data element is 16-bit, packed-128-bit)
 * cmda*_** - SIMD-data args, SIMD ISA (data element is 16-bit, packed-256-bit)
 *
 * Alphabetical view of current/future instruction namespaces is in rtzero.h.
 * Configurable BASE/SIMD subsets (cmdx*, cmdy*, cmdp*) are defined in rtconf.h.
 * Mixing of 64/32-bit fields in backend structures may lead to misalignment
 * of 64-bit fields to 4-byte boundary, which is not supported on some targets.
 * Place fields carefully to ensure natural alignment for all data types.
 * Note that within cmdx*_** subset most of the instructions follow in-heap
 * address size (RT_ADDRESS or A) and only label_ld/st, jmpxx_xr/xm follow
 * pointer size (RT_POINTER or P) as code/data/stack segments are fixed.
 * Stack ops always work with full registers regardless of the mode chosen.
 *
 * 32-bit and 64-bit BASE subsets are not easily compatible on all targets,
 * thus any register modified with 32-bit op cannot be used in 64-bit subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subsets across all targets.
 * Registers written with 64-bit op aren't always compatible with 32-bit either,
 * as m64 requires the upper half to be all 0s or all 1s for m32 arithmetic.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subsets,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (32-bit, 64-bit or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtbase.h.
 *
 * Setting-flags instruction naming scheme may change again in the future for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and POWER use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
 *
 * The cmdp*_** (rtconf.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data elements (fp+int).
 * In this model data paths are fixed-width, BASE and SIMD data elements are
 * width-compatible, code path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtconf.h as well.
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing with 512-bit
 * and wider vectors. Use of scalars may leave respective vector registers
 * undefined, as seen from the perspective of any particular vector subset.
 *
 * 256-bit vectors used with wider subsets may not be compatible with regards
 * to memory loads/stores when mixed in the code. It means that data loaded
 * with wider vector and stored within 256-bit subset at the same address may
 * result in changing the initial representation in memory. The same can be
 * said about mixing vector and scalar subsets. Scalars can be completely
 * detached on some architectures. Use elm*x_st to store 1st vector element.
 * 128-bit vectors should be memory-compatible with any wider vector subset.
 *
 * Handling of NaNs in the floating point pipeline may not be consistent
 * across different architectures. Avoid NaNs entering the data flow by using
 * masking or control flow instructions. Apply special care when dealing with
 * floating point compare and min/max input/output. The result of floating point
 * compare instructions can be considered a -QNaN, though it is also interpreted
 * as integer -1 and is often treated as a mask. Most arithmetic instructions
 * should propagate QNaNs unchanged, however this behavior hasn't been verified.
 *
 * Working with sub-word BASE elements (byte, half) is reserved for future use.
 * However, current displacement types may not work due to natural alignment.
 * Signed/unsigned types can be supported orthogonally in cmd*n_**, cmd*x_**.
 * Working with sub-word SIMD elements (byte, half) has not been investigated.
 * However, as current major ISAs lack the ability to do sub-word fp-compute,
 * these corresponding subsets cannot be viewed as valid targets for SPMD.
 *
 * Scalar SIMD improvements, horizontal SIMD reductions, wider SIMD vectors
 * with zeroing/merging predicates in 3/4-operand instructions are planned as
 * future extensions to current 2/3-operand SPMD-driven vertical SIMD ISA.
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Master config flags for SIMD compatibility modes across all targets.
 * Each target can be configured individually regardless of the master flag.
 * Refer to individual target sections below for flags' detailed description.
 * General rule for interpretation of RT_SIMD_COMPAT_* values is as following:
 * 0 - non-compat (fastest), 1 - max-compat (slower), 2 - min-compat (faster).
 */
#define RT_SIMD_COMPAT_XMM_MASTER       1 /* 0 - 16, 1 - 15, 2 - 14 regs */
#define RT_SIMD_COMPAT_RCP_MASTER       1 /* for full-precision rcpps_** */
#define RT_SIMD_COMPAT_RSQ_MASTER       1 /* for full-precision rsqps_** */
#define RT_SIMD_COMPAT_FMA_MASTER       1 /* for full-precision fmaps_** */
#define RT_SIMD_COMPAT_FMS_MASTER       1 /* for full-precision fmsps_** */
#define RT_SIMD_COMPAT_DIV_MASTER       1 /* for full-precision divps_** */
#define RT_SIMD_COMPAT_SQR_MASTER       1 /* for full-precision sqrps_** */
#define RT_SIMD_COMPAT_SSE_MASTER       4 /* for v4 slot SSE2/4.1 - 2,4 (x64) */
#define RT_SIMD_COMPAT_FMR_MASTER       0 /* for fm*ps_** rounding mode (x86) */
#define RT_SIMD_FLUSH_ZERO_MASTER       0 /* optional on MIPS and POWER */

/*
 * Determine maximum of available SIMD registers for applications' code-bases.
 */
#if   (defined RT_ARM) || (defined RT_X86) /* <- original legacy targets */
#undef  RT_REGS
#define RT_REGS 8
#endif /* RT_REGS: 8 */

#if   (defined RT_REGS)
/* RT_REGS is already defined outside */
#elif (RT_256_R8 || RT_512_R8 || RT_1K4_R8 || RT_2K8_R8)
#define RT_REGS 8       /* <- 8 on 128/256-paired/512-quaded x64 targets */
#elif ((RT_128) & 0x3C || (RT_256) & 0x03 || (RT_512) & 0x03 || (RT_1K4) & 0x03)
#define RT_REGS 16      /* <- 15 on 128-paired/quaded RISC/POWER targets */
#elif ((RT_128) & 0x03 || (RT_256) & 0x0C || (RT_512) & 0x0C || (RT_1K4) & 0x0C)
#define RT_REGS 32      /* <- 30 on all modern 128-bit SIMD RISC targets */
#elif (RT_128_RX || RT_256_RX || RT_512_RX || RT_1K4_RX)
#define RT_REGS 32      /* <- 30 on predicated x64 AVX-512/1K4 & ARM-SVE */
#endif /* RT_REGS: 8, 16, 32 */

/*
 * Short name for true-condition sign in assembler evaluation of (A == B).
 * The result of the condition evaluation is used as a mask for selection:
 * ((A == B) & C) | ((A != B) & D), therefore it needs to be (-1) if true.
 */
#if __llvm__
#define M   -
#else /* GAS */
#define M   +
#endif /* M */

/******************************************************************************/
/***************************   OS, COMPILER, ARCH   ***************************/
/******************************************************************************/

/****************************   LINUX/WIN64, GCC   ****************************/

#if   (defined RT_LINUX) || (defined RT_WIN64) /* <- only for x64 (TDM64-GCC) */

/* ---------------------------------   ARM   -------------------------------- */

#if   (defined RT_ARM) /* original legacy target, supports only 8 registers */

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2
#define ASM_OP3(op, p1, p2, p3) #op"  "#p1", "#p2", "#p3

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMPTY                   ASM_BEG ASM_END /* endian-agnostic */
#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define EMITW(w)                ASM_BEG ASM_OP1(.long, w) ASM_END

#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, r0, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, lb, r0) ASM_END

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(movw, r0, :lower16:lb) ASM_END                      \
        ASM_BEG ASM_OP2(movt, r0, :upper16:lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMITW(0xE5800000 | MDM(Teax,    MOD(MD), VAL(DD), B3(DD), P1(DD)))

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#ifndef RT_SIMD_FAST_FCTRL
#define RT_SIMD_FAST_FCTRL      1 /* takes all available regs except (SP, PC) */
#endif /* RT_SIMD_FAST_FCTRL */

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#ifndef RT_SIMD_FLUSH_ZERO
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
#endif /* RT_SIMD_FLUSH_ZERO */

/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RCP
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
#endif /* RT_SIMD_COMPAT_RCP */

/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RSQ
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
#endif /* RT_SIMD_COMPAT_RSQ */

/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMA
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
#endif /* RT_SIMD_COMPAT_FMA */

/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMS
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
#endif /* RT_SIMD_COMPAT_FMS */

/* RT_SIMD_COMPAT_DIV when enabled changes the default behavior
 * of divps_** to the corresponding IEEE-compatible fallback */
#ifndef RT_SIMD_COMPAT_DIV
#define RT_SIMD_COMPAT_DIV      RT_SIMD_COMPAT_DIV_MASTER
#endif /* RT_SIMD_COMPAT_DIV */

/* RT_SIMD_COMPAT_SQR when enabled changes the default behavior
 * of sqrps_** to the corresponding IEEE-compatible fallback */
#ifndef RT_SIMD_COMPAT_SQR
#define RT_SIMD_COMPAT_SQR      RT_SIMD_COMPAT_SQR_MASTER
#endif /* RT_SIMD_COMPAT_SQR */

/* RT_BASE_COMPAT_DIV when enabled changes the default behavior
 * of divide instructions to their native int implementation */
#ifdef  RT_SIMD_CODE
#undef  RT_BASE_COMPAT_DIV
#define RT_BASE_COMPAT_DIV 2 /* enable int-div when SIMD target is chosen */
#if   (RT_128X1 == 1) && (RT_SIMD == 128)
#undef  RT_BASE_COMPAT_DIV
#define RT_BASE_COMPAT_DIV 0 /* no int-div for Cortex-A8/A9, ARMv7+NEONv1 */
#endif /* RT_SIMD: 128 */
#endif /* RT_SIMD_CODE */

#if   (RT_2K8X1 != 0) && (RT_SIMD == 2048)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_1K4X2 != 0) && (RT_SIMD == 2048)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X4 != 0) && (RT_SIMD == 2048)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_1K4X1 != 0) && (RT_SIMD == 1024)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X2 != 0) && (RT_SIMD == 1024)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_256X4 != 0) && (RT_SIMD == 1024)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X1 != 0) && (RT_SIMD == 512)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_256X2 != 0) && (RT_SIMD == 512)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_128X4 != 0) && (RT_SIMD == 512)
#error "AArch32 doesn't support quaded SIMD backends, check build flags"
#elif (RT_256X1 != 0) && (RT_SIMD == 256)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_128X2 != 0) && (RT_SIMD == 256)
#error "AArch32 doesn't support paired SIMD backends, check build flags"
#elif (RT_128X1 >= 1) && (RT_SIMD == 128) && (RT_REGS <= 8)
#include "rtarch_arm_128x1v4.h"
#else  /* report an error if header file is not selected */
#error "couldn't select appropriate SIMD target, check build flags"
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The SIMD unit is set to operate in its default mode (non-IEEE on ARMv7).
 */

#if RT_SIMD_FLUSH_ZERO == 0
#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITW(0xE3A00500 | MRM(TNxx, 0x00, 0x00)) /* r8  <- (0 << 22) */

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITW(0xE3A00503 | MRM(TExx, 0x00, 0x00)) /* r14 <- (3 << 22) */    \
        EMITW(0xE3A00502 | MRM(TCxx, 0x00, 0x00)) /* r12 <- (2 << 22) */    \
        EMITW(0xE3A00501 | MRM(TAxx, 0x00, 0x00)) /* r10 <- (1 << 22) */    \
        EMITW(0xE3A00500 | MRM(TNxx, 0x00, 0x00)) /* r8  <- (0 << 22) */

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */
#else /* RT_SIMD_FLUSH_ZERO */

#define ASM_ENTER(__Info__) ASM_ENTER_F(__Info__)

#define ASM_LEAVE(__Info__) ASM_LEAVE_F(__Info__)

#endif /* RT_SIMD_FLUSH_ZERO */

/*
 * The ASM_ENTER_F/ASM_LEAVE_F versions share the traits of the original ones,
 * except that they put the SIMD unit into slightly faster non-IEEE mode,
 * where denormal results from floating point operations are flushed to zero.
 * This mode is closely compatible with ARMv7, which lacks full IEEE support.
 */

#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITW(0xE3A00504 | MRM(TNxx, 0x00, 0x00)) /* r8  <- (4 << 22) */    \
        EMITW(0xEEE10A10 | MRM(TNxx, 0x00, 0x00)) /* fpscr <- r8 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0xE3A00500 | MRM(TNxx, 0x00, 0x00)) /* r8  <- (0 << 22) */    \
        EMITW(0xEEE10A10 | MRM(TNxx, 0x00, 0x00)) /* fpscr <- r8 */         \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITW(0xE3A00507 | MRM(TExx, 0x00, 0x00)) /* r14 <- (7 << 22) */    \
        EMITW(0xE3A00506 | MRM(TCxx, 0x00, 0x00)) /* r12 <- (6 << 22) */    \
        EMITW(0xE3A00505 | MRM(TAxx, 0x00, 0x00)) /* r10 <- (5 << 22) */    \
        EMITW(0xE3A00504 | MRM(TNxx, 0x00, 0x00)) /* r8  <- (4 << 22) */    \
        EMITW(0xEEE10A10 | MRM(TNxx, 0x00, 0x00)) /* fpscr <- r8 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0xE3A00500 | MRM(TNxx, 0x00, 0x00)) /* r8  <- (0 << 22) */    \
        EMITW(0xEEE10A10 | MRM(TNxx, 0x00, 0x00)) /* fpscr <- r8 */         \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef RT_SIMD_CODE
#define sregs_sa()
#define sregs_la()
#endif /* RT_SIMD_CODE */

/* ------------------------------   A32, A64   ------------------------------ */

#elif (defined RT_A32) || (defined RT_A64)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2
#define ASM_OP3(op, p1, p2, p3) #op"  "#p1", "#p2", "#p3

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMPTY                   ASM_BEG ASM_END /* endian-agnostic */
#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define EMITW(w)                ASM_BEG ASM_OP1(.long, w) ASM_END

#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, x0, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, lb, x0) ASM_END

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(adrp, x0, lb) ASM_END                               \
        ASM_BEG ASM_OP3(add,  x0, x0, :lo12:lb) ASM_END

#if   (defined RT_A32)

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xB8000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#elif (defined RT_A64)

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xF8000000 | MDM(Teax,    MOD(MD), VXL(DD), B1(DD), P1(DD)))

#endif /* defined (RT_A32, RT_A64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#ifndef RT_SIMD_FAST_FCTRL
#define RT_SIMD_FAST_FCTRL      1
#endif /* RT_SIMD_FAST_FCTRL */

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#ifndef RT_SIMD_FLUSH_ZERO
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
#endif /* RT_SIMD_FLUSH_ZERO */

/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#ifndef RT_SIMD_COMPAT_XMM
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
#endif /* RT_SIMD_COMPAT_XMM */

/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RCP
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
#endif /* RT_SIMD_COMPAT_RCP */

/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RSQ
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
#endif /* RT_SIMD_COMPAT_RSQ */

/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMA
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
#endif /* RT_SIMD_COMPAT_FMA */

/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMS
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
#endif /* RT_SIMD_COMPAT_FMS */

#if   (RT_2K8X1 >= 1) && (RT_SIMD == 2048) && (RT_REGS <= 32)
#include "rtarch_a64_SVEx1v1.h"
#elif (RT_1K4X2 != 0) && (RT_SIMD == 2048) && (RT_REGS <= 16)
#include "rtarch_a64_SVEx2v1.h"
#elif (RT_512X4 != 0) && (RT_SIMD == 2048)
#error "AArch64 doesn't support quaded SIMD backends, check build flags"
#elif (RT_1K4X1 >= 1) && (RT_SIMD == 1024) && (RT_REGS <= 32)
#include "rtarch_a64_SVEx1v1.h"
#elif (RT_512X2 != 0) && (RT_SIMD == 1024) && (RT_REGS <= 16)
#include "rtarch_a64_SVEx2v1.h"
#elif (RT_256X4 != 0) && (RT_SIMD == 1024)
#error "AArch64 doesn't support quaded SIMD backends, check build flags"
#elif (RT_512X1 >= 1) && (RT_SIMD == 512) && (RT_REGS <= 32)
#include "rtarch_a64_SVEx1v1.h"
#elif (RT_256X2 != 0) && (RT_SIMD == 512) && (RT_REGS <= 16)
#include "rtarch_a64_SVEx2v1.h"
#elif (RT_128X4 != 0) && (RT_SIMD == 512)
#error "AArch64 doesn't support quaded SIMD backends, check build flags"
#elif (RT_256X1 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 32)
#include "rtarch_a64_SVEx1v1.h"
#elif (RT_128X2 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 16)
#include "rtarch_a64_128x2v1.h"
#elif (RT_128X1 >= 1) && (RT_SIMD == 128) && (RT_REGS <= 32)
#include "rtarch_a64_128x1v1.h"
#else  /* report an error if header file is not selected */
#error "couldn't select appropriate SIMD target, check build flags"
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The SIMD unit is set to operate in its default mode (non-IEEE on ARMv7).
 */

#if RT_SIMD_FLUSH_ZERO == 0
#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x2518E3E0)                    /* SVE: p0  <- all-ones */     \
        movpx_ld(XmmE, Mebp, inf_GPC07)      /* SVE: z14 <- all-ones */     \
        EMITS(0x04603000 | MXM(TmmQ, 0x0E, 0x0E)) /* z15 <- z14 (or) */     \
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* x20 <- (0 << 22) */

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x2518E3E0)                    /* SVE: p0  <- all-ones */     \
        movpx_ld(XmmE, Mebp, inf_GPC07)      /* SVE: z14 <- all-ones */     \
        EMITS(0x04603000 | MXM(TmmQ, 0x0E, 0x0E)) /* z15 <- z14 (or) */     \
        EMITW(0x52A01800 | MRM(TExx, 0x00, 0x00)) /* x23 <- (3 << 22) */    \
        EMITW(0x52A01000 | MRM(TCxx, 0x00, 0x00)) /* x22 <- (2 << 22) */    \
        EMITW(0x52A00800 | MRM(TAxx, 0x00, 0x00)) /* x21 <- (1 << 22) */    \
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* x20 <- (0 << 22) */

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */
#else /* RT_SIMD_FLUSH_ZERO */

#define ASM_ENTER(__Info__) ASM_ENTER_F(__Info__)

#define ASM_LEAVE(__Info__) ASM_LEAVE_F(__Info__)

#endif /* RT_SIMD_FLUSH_ZERO */

/*
 * The ASM_ENTER_F/ASM_LEAVE_F versions share the traits of the original ones,
 * except that they put the SIMD unit into slightly faster non-IEEE mode,
 * where denormal results from floating point operations are flushed to zero.
 * This mode is closely compatible with ARMv7, which lacks full IEEE support.
 */

#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x2518E3E0)                    /* SVE: p0  <- all-ones */     \
        movpx_ld(XmmE, Mebp, inf_GPC07)      /* SVE: z14 <- all-ones */     \
        EMITS(0x04603000 | MXM(TmmQ, 0x0E, 0x0E)) /* z15 <- z14 (or) */     \
        EMITW(0x52A02000 | MRM(TNxx, 0x00, 0x00)) /* x20 <- (4 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- x20 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* x20 <- (0 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- x20 */         \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x2518E3E0)                    /* SVE: p0  <- all-ones */     \
        movpx_ld(XmmE, Mebp, inf_GPC07)      /* SVE: z14 <- all-ones */     \
        EMITS(0x04603000 | MXM(TmmQ, 0x0E, 0x0E)) /* z15 <- z14 (or) */     \
        EMITW(0x52A03800 | MRM(TExx, 0x00, 0x00)) /* x23 <- (7 << 22) */    \
        EMITW(0x52A03000 | MRM(TCxx, 0x00, 0x00)) /* x22 <- (6 << 22) */    \
        EMITW(0x52A02800 | MRM(TAxx, 0x00, 0x00)) /* x21 <- (5 << 22) */    \
        EMITW(0x52A02000 | MRM(TNxx, 0x00, 0x00)) /* x20 <- (4 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- x20 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* x20 <- (0 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- x20 */         \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef RT_SIMD_CODE
#define sregs_sa()
#define sregs_la()
#define movpx_ld(XD, MS, DS)
#define EMITS(w) /* EMPTY */
#elif (defined RT_SVEX1) || (defined RT_SVEX2)
#define EMITS(w)    EMITW(w)
#else
#define EMITS(w) /* EMPTY */
#endif /* RT_SIMD_CODE */

#if   (defined RT_SVEX1) || (defined RT_SVEX2)
#define EMITV(w)    EMITW(w)
#else
#define EMITV(w) /* EMPTY */
#endif /* RT_SVEX1, RT_SVEX2 */

/* ------------------------------   M32, M64   ------------------------------ */

#elif (defined RT_M32) || (defined RT_M64)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2
#define ASM_OP3(op, p1, p2, p3) #op"  "#p1", "#p2", "#p3

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMPTY                   ASM_BEG ASM_END /* endian-agnostic */
#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define EMITW(w)                ASM_BEG ASM_OP1(.long, w) ASM_END

#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(move, $a0, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(move, lb, $a0) ASM_END

#if   (defined RT_M32)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(la, $a0, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMITW(0xAC000000 | MDM(Teax,    MOD(MD), VAL(DD), B3(DD), P1(DD)))

#elif (defined RT_M64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(dla, $a0, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMITW(0xFC000000 | MDM(Teax,    MOD(MD), VAL(DD), B3(DD), P1(DD)))

#endif /* defined (RT_M32, RT_M64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#ifndef RT_SIMD_FAST_FCTRL
#define RT_SIMD_FAST_FCTRL      1
#endif /* RT_SIMD_FAST_FCTRL */

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#ifndef RT_SIMD_FLUSH_ZERO
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
#endif /* RT_SIMD_FLUSH_ZERO */

/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#ifndef RT_SIMD_COMPAT_XMM
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
#endif /* RT_SIMD_COMPAT_XMM */

/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RCP
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
#endif /* RT_SIMD_COMPAT_RCP */

/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RSQ
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
#endif /* RT_SIMD_COMPAT_RSQ */

/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMA
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
#endif /* RT_SIMD_COMPAT_FMA */

/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMS
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
#endif /* RT_SIMD_COMPAT_FMS */

/* RT_SIMD_COMPAT_D12 when enabled uses fp64 load/store in SIMD
 * to make DP displacement type 12-bit for fp32, 11-bit if 0 */
#ifndef RT_SIMD_COMPAT_D12
#define RT_SIMD_COMPAT_D12      (Q/RT_DATA != 0) /* if DP is filled over half */
#endif /* RT_SIMD_COMPAT_D12 */

/* RT_ELEM_COMPAT_MSA when enabled makes scalar SIMD (ELEM) ops
 * compatible with MSA, applicable to big-endian vector fp32 */
#ifndef RT_ELEM_COMPAT_MSA
#define RT_ELEM_COMPAT_MSA      0 /* applicable to MIPS fp32 */
#endif /* RT_ELEM_COMPAT_MSA */

/* RT_BASE_COMPAT_REV when enabled changes the default behavior
 * of integer instructions to their MIPS Revision 6 variants */
#undef  RT_BASE_COMPAT_REV
#define RT_BASE_COMPAT_REV (RT_M32 | RT_M64) /* enable MIPS Revision 6 */

#if   (RT_2K8X1 != 0) && (RT_SIMD == 2048)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_1K4X2 != 0) && (RT_SIMD == 2048)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X4 != 0) && (RT_SIMD == 2048)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_1K4X1 != 0) && (RT_SIMD == 1024)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X2 != 0) && (RT_SIMD == 1024)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_256X4 != 0) && (RT_SIMD == 1024)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X1 != 0) && (RT_SIMD == 512)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_256X2 != 0) && (RT_SIMD == 512)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_128X4 != 0) && (RT_SIMD == 512)
#error "mipsMSA doesn't support quaded SIMD backends, check build flags"
#elif (RT_256X1 != 0) && (RT_SIMD == 256)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_128X2 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 16)
#include "rtarch_m64_128x2v1.h"
#elif (RT_128X1 >= 1) && (RT_SIMD == 128) && (RT_REGS <= 32)
#include "rtarch_m64_128x1v1.h"
#else  /* report an error if header file is not selected */
#error "couldn't select appropriate SIMD target, check build flags"
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The SIMD unit is set to operate in its default mode (non-IEEE on ARMv7).
 */

#if RT_SIMD_FLUSH_ZERO == 0
#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x7860001E | MXM(TmmZ, TmmZ, TmmZ)) /* w30 <- 0 (xor) */      \
        EMITW(0x3C000000 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(0 << 24) */

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x7860001E | MXM(TmmZ, TmmZ, TmmZ)) /* w30 <- 0 (xor) */      \
        EMITW(0x34000003 | MRM(0x00, TZxx, TExx)) /* r23 <- 3|(0 << 24) */  \
        EMITW(0x34000002 | MRM(0x00, TZxx, TCxx)) /* r22 <- 2|(0 << 24) */  \
        EMITW(0x34000001 | MRM(0x00, TZxx, TAxx)) /* r21 <- 1|(0 << 24) */  \
        EMITW(0x3C000000 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(0 << 24) */

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */
#else /* RT_SIMD_FLUSH_ZERO */

#define ASM_ENTER(__Info__) ASM_ENTER_F(__Info__)

#define ASM_LEAVE(__Info__) ASM_LEAVE_F(__Info__)

#endif /* RT_SIMD_FLUSH_ZERO */

/*
 * The ASM_ENTER_F/ASM_LEAVE_F versions share the traits of the original ones,
 * except that they put the SIMD unit into slightly faster non-IEEE mode,
 * where denormal results from floating point operations are flushed to zero.
 * This mode is closely compatible with ARMv7, which lacks full IEEE support.
 */

#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x7860001E | MXM(TmmZ, TmmZ, TmmZ)) /* w30 <- 0 (xor) */      \
        EMITW(0x3C000100 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(1 << 24) */  \
        EMITW(0x44C0F800 | MRM(0x00, 0x00, TNxx)) /* fcsr <- r20 */         \
        EMITS(0x783E0059 | MXM(0x00, TNxx, 0x00)) /* msacsr <- r20 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0x3C000000 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(0 << 24) */  \
        EMITW(0x44C0F800 | MRM(0x00, 0x00, TNxx)) /* fcsr <- r20 */         \
        EMITS(0x783E0059 | MXM(0x00, TNxx, 0x00)) /* msacsr <- r20 */       \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        EMITS(0x7860001E | MXM(TmmZ, TmmZ, TmmZ)) /* w30 <- 0 (xor) */      \
        EMITW(0x34000003 | MRM(0x00, TZxx, TExx)) /* r23 <- 3|(1 << 24) */  \
        EMITW(0x34000002 | MRM(0x00, TZxx, TCxx)) /* r22 <- 2|(1 << 24) */  \
        EMITW(0x34000001 | MRM(0x00, TZxx, TAxx)) /* r21 <- 1|(1 << 24) */  \
        EMITW(0x3C000100 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(1 << 24) */  \
        EMITW(0x44C0F800 | MRM(0x00, 0x00, TNxx)) /* fcsr <- r20 */         \
        EMITS(0x783E0059 | MXM(0x00, TNxx, 0x00)) /* msacsr <- r20 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0x3C000000 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(0 << 24) */  \
        EMITW(0x44C0F800 | MRM(0x00, 0x00, TNxx)) /* fcsr <- r20 */         \
        EMITS(0x783E0059 | MXM(0x00, TNxx, 0x00)) /* msacsr <- r20 */       \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef RT_SIMD_CODE
#define sregs_sa()
#define sregs_la()
#define EMITS(w) /* EMPTY */
#else  /* RT_SIMD_CODE */
#define EMITS(w)    EMITW(w)
#endif /* RT_SIMD_CODE */

/* ------------------------------   P32, P64   ------------------------------ */

#elif (defined RT_P32) || (defined RT_P64)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2
#define ASM_OP3(op, p1, p2, p3) #op"  "#p1", "#p2", "#p3

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMPTY                   ASM_BEG ASM_END /* endian-agnostic */
#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define EMITW(w)                ASM_BEG ASM_OP1(.long, w) ASM_END

#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(mr, %%r4, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(mr, lb, %%r4) ASM_END

#if   (defined RT_P32)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(lis, %%r4, lb@h) ASM_END                            \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@l) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), O1(DD)))

#elif (defined RT_P64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(lis, %%r4, lb@highest) ASM_END                      \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@higher) ASM_END                 \
        ASM_BEG ASM_OP3(sldi, %%r4, %%r4, 32) ASM_END                       \
        ASM_BEG ASM_OP3(oris, %%r4, %%r4, lb@h) ASM_END                     \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@l) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), Q1(DD)))

#endif /* defined (RT_P32, RT_P64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#ifndef RT_SIMD_FAST_FCTRL
#define RT_SIMD_FAST_FCTRL      1 /* not applicable to POWER */
#endif /* RT_SIMD_FAST_FCTRL */

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#ifndef RT_SIMD_FLUSH_ZERO
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
#endif /* RT_SIMD_FLUSH_ZERO */

/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#ifndef RT_SIMD_COMPAT_XMM
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
#endif /* RT_SIMD_COMPAT_XMM */

/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RCP
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
#endif /* RT_SIMD_COMPAT_RCP */

/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RSQ
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
#endif /* RT_SIMD_COMPAT_RSQ */

/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMA
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
#endif /* RT_SIMD_COMPAT_FMA */

/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMS
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
#endif /* RT_SIMD_COMPAT_FMS */

/* RT_SIMD_COMPAT_DIV when enabled changes the default behavior
 * of divps_** to the corresponding IEEE-compatible fallback */
#ifndef RT_SIMD_COMPAT_DIV
#define RT_SIMD_COMPAT_DIV      RT_SIMD_COMPAT_DIV_MASTER
#endif /* RT_SIMD_COMPAT_DIV */

/* RT_SIMD_COMPAT_SQR when enabled changes the default behavior
 * of sqrps_** to the corresponding IEEE-compatible fallback */
#ifndef RT_SIMD_COMPAT_SQR
#define RT_SIMD_COMPAT_SQR      RT_SIMD_COMPAT_SQR_MASTER
#endif /* RT_SIMD_COMPAT_SQR */

/* RT_SIMD_COMPAT_PW8 when enabled picks IBM POWER8 ISA variant
 * on top of default POWER7 ISA, only POWER8 is LE-qualified */
#ifndef RT_SIMD_COMPAT_PW8
#define RT_SIMD_COMPAT_PW8      (1 - RT_ENDIAN)
#endif /* RT_SIMD_COMPAT_PW8 */

/* RT_SIMD_COMPAT_VSX when enabled replaces default VMX targets
 * with regular VSX implementation (RT_128=4+8, RT_256_R8=4) */
#ifndef RT_SIMD_COMPAT_VSX
#define RT_SIMD_COMPAT_VSX      0 /* applicable to POWER+VSX */
#endif /* RT_SIMD_COMPAT_VSX */

/* RT_ELEM_COMPAT_VMX when enabled makes scalar SIMD (ELEM) ops
 * compatible with VMX, only if BASE regs are 128bit-aligned */
#ifndef RT_ELEM_COMPAT_VMX
#define RT_ELEM_COMPAT_VMX      0 /* applicable to all POWER */
#endif /* RT_ELEM_COMPAT_VMX */

/* RT_BASE_COMPAT_ZFL when enabled makes setting-flags BASE ops
 * compatible with 64-bit processors running 32-bit ISA mode */
#ifndef RT_BASE_COMPAT_ZFL
#define RT_BASE_COMPAT_ZFL      1 /* only necessary on POWER */
#endif /* RT_BASE_COMPAT_ZFL */

#if   (RT_2K8X1 != 0) && (RT_SIMD == 2048)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_1K4X2 != 0) && (RT_SIMD == 2048)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X4 != 0) && (RT_SIMD == 2048)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_1K4X1 != 0) && (RT_SIMD == 1024)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X2 != 0) && (RT_SIMD == 1024)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_256X4 != 0) && (RT_SIMD == 1024)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_512X1 != 0) && (RT_SIMD == 512)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_256X2 != 0) && (RT_SIMD == 512)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_128X4 >= 4) && (RT_SIMD == 512) && (RT_REGS <= 32)
#error "PowerPC doesn't support 128x4 SIMD with 30-regs, check build flags"
#elif (RT_128X4 >= 2) && (RT_SIMD == 512) && (RT_REGS <= 16)
#include "rtarch_p64_128x4v2.h"
#elif (RT_128X4 >= 1) && (RT_SIMD == 512) && (RT_REGS <= 16)
#include "rtarch_p64_128x4v1.h"
#elif (RT_256X1 != 0) && (RT_SIMD == 256)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#elif (RT_128X2 & 16) && (RT_SIMD == 256) && (RT_REGS <= 8)
#if (RT_SIMD_COMPAT_VSX != 0)
#include "rtarch_p64_128x2v1.h"
#else  /* RT_SIMD_COMPAT_VSX */
#include "rtarch_p32_128x2vG.h"
#endif /* RT_SIMD_COMPAT_VSX */
#elif (RT_128X2 >= 8) && (RT_SIMD == 256) && (RT_REGS <= 32)
#include "rtarch_p64_128x2v8.h"
#elif (RT_128X2 >= 4) && (RT_SIMD == 256) && (RT_REGS <= 32)
#include "rtarch_p64_128x2v4.h"
#elif (RT_128X2 >= 2) && (RT_SIMD == 256) && (RT_REGS <= 16)
#include "rtarch_p64_128x2v2.h"
#elif (RT_128X2 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 16)
#include "rtarch_p64_128x2v1.h"
#elif (RT_128X1 >= 8) && (RT_SIMD == 128) && (RT_REGS <= 16)
#if (RT_SIMD_COMPAT_VSX != 0)
#include "rtarch_p64_128x1v2.h"
#else  /* RT_SIMD_COMPAT_VSX */
#include "rtarch_p32_128x1v4.h"
#endif /* RT_SIMD_COMPAT_VSX */
#elif (RT_128X1 >= 4) && (RT_SIMD == 128) && (RT_REGS <= 16)
#if (RT_SIMD_COMPAT_VSX != 0)
#include "rtarch_p64_128x1v1.h"
#else  /* RT_SIMD_COMPAT_VSX */
#include "rtarch_p32_128x1v4.h"
#endif /* RT_SIMD_COMPAT_VSX */
#elif (RT_128X1 >= 2) && (RT_SIMD == 128) && (RT_REGS <= 32)
#include "rtarch_p64_128x1v2.h"
#elif (RT_128X1 >= 1) && (RT_SIMD == 128) && (RT_REGS <= 32)
#include "rtarch_p64_128x1v1.h"
#else  /* report an error if header file is not selected */
#error "couldn't select appropriate SIMD target, check build flags"
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The SIMD unit is set to operate in its default mode (non-IEEE on ARMv7).
 */

#if RT_SIMD_FLUSH_ZERO == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        EMITS(0x38000000 | MRM(T0xx, 0x00, 0x00)) /* r20 <- 0 */            \
        EMITS(0x38000010 | MRM(T1xx, 0x00, 0x00)) /* r21 <- 16 */           \
        EMITS(0x38000020 | MRM(T2xx, 0x00, 0x00)) /* r22 <- 32 */           \
        EMITS(0x38000030 | MRM(T3xx, 0x00, 0x00)) /* r23 <- 48 */           \
        EMITW(0x7C000278 | MSM(TZxx, TZxx, TZxx)) /* r0  <- 0 (xor) */      \
        sregs_sa()                                                          \
        EMITW(0x7C000040 | MRM(0x08, TLxx, TLxx)) /* cmplw cr2, r24, r24 */ \
        EMITW(0x7C0002A6 | MRM(TCxx, 0x00, 0x09)) /* ctr -> r28 */          \
        EMITS(0x7C0002A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave -> r29 */       \
        EMITS(0x3800FFFF | MRM(TIxx, 0x00, 0x00)) /* r25 <- -1 */           \
        EMITS(0x7C0003A6 | MRM(TIxx, 0x08, 0x00)) /* vrsave <- r25 */       \
        EMITS(0x1000038C | MXM(TmmQ, 0x1F, 0x00)) /* v15 <- all-ones */     \
        movix_ld(Xmm2, Mebp, inf_GPC01_32)        /* v2  <- +1.0f 32-bit */ \
        movix_ld(Xmm4, Mebp, inf_GPC02_32)        /* v4  <- -0.5f 32-bit */ \
        movix_ld(Xmm8, Mebp, inf_GPC04_32)        /* v8  <- 0x7FFFFFFF */   \
        EMITM(0x100004C4 | MXM(TmmR, TmmR, TmmR)) /* v24 <- v24 xor v24 */  \
        EMITM(0x10000504 | MXM(TmmS, 0x08, 0x08)) /* v25 <- not v8 */       \
        EMITM(0x10000484 | MXM(TmmU, 0x02, 0x02)) /* v26 <- v2 */           \
        EMITM(0x10000484 | MXM(TmmV, 0x04, 0x04)) /* v27 <- v4 */           \
        EMITP(0xF0000496 | MXM(TmmQ, 0x02, 0x02)) /* vs15 <- v2 */          \
        EMITP(0xF0000496 | MXM(TmmM, 0x04, 0x04)) /* vs31 <- v4 */

#define ASM_LEAVE(__Info__)                                                 \
        EMITW(0x7C0003A6 | MRM(TCxx, 0x00, 0x09)) /* ctr <- r28 */          \
        EMITS(0x7C0003A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave <- r29 */       \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FLUSH_ZERO */

#define ASM_ENTER(__Info__) ASM_ENTER_F(__Info__)

#define ASM_LEAVE(__Info__) ASM_LEAVE_F(__Info__)

#endif /* RT_SIMD_FLUSH_ZERO */

/*
 * The ASM_ENTER_F/ASM_LEAVE_F versions share the traits of the original ones,
 * except that they put the SIMD unit into slightly faster non-IEEE mode,
 * where denormal results from floating point operations are flushed to zero.
 * This mode is closely compatible with ARMv7, which lacks full IEEE support.
 */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        EMITS(0x38000000 | MRM(T0xx, 0x00, 0x00)) /* r20 <- 0 */            \
        EMITS(0x38000010 | MRM(T1xx, 0x00, 0x00)) /* r21 <- 16 */           \
        EMITS(0x38000020 | MRM(T2xx, 0x00, 0x00)) /* r22 <- 32 */           \
        EMITS(0x38000030 | MRM(T3xx, 0x00, 0x00)) /* r23 <- 48 */           \
        EMITW(0x7C000278 | MSM(TZxx, TZxx, TZxx)) /* r0  <- 0 (xor) */      \
        sregs_sa()                                                          \
        EMITW(0x7C000040 | MRM(0x08, TLxx, TLxx)) /* cmplw cr2, r24, r24 */ \
        EMITW(0x7C0002A6 | MRM(TCxx, 0x00, 0x09)) /* ctr -> r28 */          \
        EMITS(0x7C0002A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave -> r29 */       \
        EMITS(0x3800FFFF | MRM(TIxx, 0x00, 0x00)) /* r25 <- -1 */           \
        EMITS(0x7C0003A6 | MRM(TIxx, 0x08, 0x00)) /* vrsave <- r25 */       \
        EMITS(0x1000038C | MXM(TmmQ, 0x1F, 0x00)) /* v15 <- all-ones */     \
        movix_ld(Xmm2, Mebp, inf_GPC01_32)        /* v2  <- +1.0f 32-bit */ \
        movix_ld(Xmm4, Mebp, inf_GPC02_32)        /* v4  <- -0.5f 32-bit */ \
        movix_ld(Xmm8, Mebp, inf_GPC04_32)        /* v8  <- 0x7FFFFFFF */   \
        EMITM(0x100004C4 | MXM(TmmR, TmmR, TmmR)) /* v24 <- v24 xor v24 */  \
        EMITM(0x10000504 | MXM(TmmS, 0x08, 0x08)) /* v25 <- not v8 */       \
        EMITM(0x10000484 | MXM(TmmU, 0x02, 0x02)) /* v26 <- v2 */           \
        EMITM(0x10000484 | MXM(TmmV, 0x04, 0x04)) /* v27 <- v4 */           \
        EMITP(0xF0000496 | MXM(TmmQ, 0x02, 0x02)) /* vs15 <- v2 */          \
        EMITP(0xF0000496 | MXM(TmmM, 0x04, 0x04)) /* vs31 <- v4 */          \
        EMITW(0xFC00010C | MRM(0x1C, 0x08, 0x00)) /* fpscr <- NI(4) */      \
        EMITS(0x1000034C | MXM(TmmM, 0x01, 0x00)) /* v31 <- splt-half(1) */ \
        EMITS(0x10000644 | MXM(0x00, 0x00, TmmM)) /* vscr <- v31, NJ(16) */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0xFC00010C | MRM(0x1C, 0x00, 0x00)) /* fpscr <- NI(0) */      \
        EMITS(0x1000034C | MXM(TmmM, 0x00, 0x00)) /* v31 <- splt-half(0) */ \
        EMITS(0x10000644 | MXM(0x00, 0x00, TmmM)) /* vscr <- v31, NJ(16) */ \
        EMITW(0x7C0003A6 | MRM(TCxx, 0x00, 0x09)) /* ctr <- r28 */          \
        EMITS(0x7C0003A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave <- r29 */       \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#ifndef RT_SIMD_CODE
#define sregs_sa()
#define sregs_la()
#define movix_ld(XD, MS, DS)
#define EMITS(w) /* EMPTY */
#define EMITM(w) /* EMPTY */
#define EMITP(w) /* EMPTY */
#elif (RT_128X1 >= 4 || RT_128X2 == 16) && (RT_SIMD_COMPAT_VSX == 0)
#define EMITS(w)    EMITW(w)
#define EMITM(w)    EMITW(w)
#define EMITP(w) /* EMPTY */
#else /* RT_128X1 <= 2 || RT_128X2 <= 8 || RT_128X4 <= 2 || SIMD_COMPAT_VSX */
#define EMITS(w)    EMITW(w)
#define EMITM(w) /* EMPTY */
#define EMITP(w)    EMITW(w)
#endif /* RT_SIMD_CODE */

/* ------------------------------   X32, X64   ------------------------------ */

#elif (defined RT_X32) || (defined RT_X64)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p2", "#p1
#define ASM_OP3(op, p1, p2, p3) #op"  "#p3", "#p2", "#p1

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMPTY                   ASM_BEG ASM_END /* endian-agnostic */
#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define EMITW(w)                ASM_BEG ASM_OP1(.long, w) ASM_END

#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(movq, %%rax, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(movq, lb, %%rax) ASM_END

#if   (defined RT_X32)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(leaq, %%rax, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
    ADR REX(0,       RXB(MD)) EMITB(0x89)                                   \
        MRM(0x00,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#elif (defined RT_X64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(leaq, %%rax, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
    ADR REW(0,       RXB(MD)) EMITB(0x89)                                   \
        MRM(0x00,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#endif /* defined (RT_X32, RT_X64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#ifndef RT_SIMD_FAST_FCTRL
#define RT_SIMD_FAST_FCTRL      (Q/2 != 0) /* if build includes wider targets */
#endif /* RT_SIMD_FAST_FCTRL */

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#ifndef RT_SIMD_FLUSH_ZERO
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
#endif /* RT_SIMD_FLUSH_ZERO */

/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#ifndef RT_SIMD_COMPAT_XMM
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
#endif /* RT_SIMD_COMPAT_XMM */

/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RCP
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
#endif /* RT_SIMD_COMPAT_RCP */

/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RSQ
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
#endif /* RT_SIMD_COMPAT_RSQ */

/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMA
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
#endif /* RT_SIMD_COMPAT_FMA */

/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMS
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
#endif /* RT_SIMD_COMPAT_FMS */

/* RT_SIMD_COMPAT_SSE distinguishes between 128-bit SSE2 & SSE4
 * if RT_128 = 4 SIMD backend is present among build targets */
#ifndef RT_SIMD_COMPAT_SSE
#define RT_SIMD_COMPAT_SSE      RT_SIMD_COMPAT_SSE_MASTER
#endif /* RT_SIMD_COMPAT_SSE */

/* RT_SIMD_COMPAT_FMR when enabled changes the default behavior
 * of fm*ps_** instruction fallbacks to honour rounding mode */
#ifndef RT_SIMD_COMPAT_FMR
#define RT_SIMD_COMPAT_FMR      RT_SIMD_COMPAT_FMR_MASTER
#endif /* RT_SIMD_COMPAT_FMR */

/* RT_BASE_COMPAT_BMI when enabled changes the default behavior
 * of some bit-manipulation instructions to use BMI variants */
#ifdef  RT_SIMD_CODE
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 2 /* enable BMI1+BMI2 when SIMD target is chosen */
#if   (RT_256X2 == 1) && (RT_SIMD == 512)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#elif (RT_256X1 == 1) && (RT_SIMD == 256)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#elif (RT_256X1 == 4) && (RT_SIMD == 256)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#elif (RT_128X2 == 4) && (RT_SIMD == 256)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#elif (RT_128X1 == 2) && (RT_SIMD == 128)
#elif (RT_128X1 < 32) && (RT_SIMD == 128)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#endif /* RT_SIMD: 512, 256, 128 */
#endif /* RT_SIMD_CODE */

#if   (RT_2K8X1 != 0) && (RT_SIMD == 2048)
#error "x86_64: doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_1K4X2 != 0) && (RT_SIMD == 2048)
#error "x86_64: doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_512X4 >= 1) && (RT_SIMD == 2048) && (RT_REGS <= 8)
#include "rtarch_x64_512x4v2.h"
#elif (RT_1K4X1 != 0) && (RT_SIMD == 1024)
#error "x86_64: doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_512X2 >= 1) && (RT_SIMD == 1024) && (RT_REGS <= 16)
#include "rtarch_x64_512x2v2.h"
#elif (RT_256X4 != 0) && (RT_SIMD == 1024)
#error "x86_64: doesn't support quaded AVXx backends, check build flags"
#elif (RT_512X1 >= 1) && (RT_SIMD == 512) && (RT_REGS <= 32)
#include "rtarch_x64_512x1v8.h"
#elif (RT_256X2 >= 1) && (RT_SIMD == 512) && (RT_REGS <= 8)
#include "rtarch_x64_256x2v2.h"
#elif (RT_128X4 != 0) && (RT_SIMD == 512)
#error "x86_64: doesn't support quaded SSEx backends, check build flags"
#elif (RT_256X1 &  4) && (RT_SIMD == 256) && (RT_REGS <= 32)
#error "x86_64: doesn't yet support emulated 30-regs, check build flags"
#elif (RT_256X1 >= 8) && (RT_SIMD == 256) && (RT_REGS <= 32)
#include "rtarch_x64_256x1v8.h"
#elif (RT_256X1 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 16)
#include "rtarch_x64_256x1v2.h"
#elif (RT_128X2 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 8)
#include "rtarch_x64_128x2v4.h"
#elif (RT_128X1 &  1) && (RT_SIMD == 128) && (RT_REGS <= 32)
#error "x86_64: doesn't yet support emulated 30-regs, check build flags"
#elif (RT_128X1 >= 8) && (RT_SIMD == 128) && (RT_REGS <= 16)
#include "rtarch_x64_128x1v8.h"
#elif (RT_128X1 >= 4) && (RT_SIMD == 128) && (RT_REGS <= 16)
#include "rtarch_x64_128x1v4.h"
#elif (RT_128X1 >= 2) && (RT_SIMD == 128) && (RT_REGS <= 32)
#include "rtarch_x64_128x1v2.h"
#else  /* report an error if header file is not selected */
#error "couldn't select appropriate SIMD target, check build flags"
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The SIMD unit is set to operate in its default mode (non-IEEE on ARMv7).
 */

#if RT_SIMD_FLUSH_ZERO == 0
#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(3*4), IH(0x7F80))                          \
        movwx_mi(Mebp, inf_FCTRL(2*4), IH(0x5F80))                          \
        movwx_mi(Mebp, inf_FCTRL(1*4), IH(0x3F80))                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */
#else /* RT_SIMD_FLUSH_ZERO */

#define ASM_ENTER(__Info__) ASM_ENTER_F(__Info__)

#define ASM_LEAVE(__Info__) ASM_LEAVE_F(__Info__)

#endif /* RT_SIMD_FLUSH_ZERO */

/*
 * The ASM_ENTER_F/ASM_LEAVE_F versions share the traits of the original ones,
 * except that they put the SIMD unit into slightly faster non-IEEE mode,
 * where denormal results from floating point operations are flushed to zero.
 * This mode is closely compatible with ARMv7, which lacks full IEEE support.
 */

#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)                                               \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))                                      \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_full __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(3*4), IH(0xFF80))                          \
        movwx_mi(Mebp, inf_FCTRL(2*4), IH(0xDF80))                          \
        movwx_mi(Mebp, inf_FCTRL(1*4), IH(0xBF80))                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)                                               \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))                                      \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_full)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef RT_SIMD_CODE
#define sregs_sa()
#define sregs_la()
#define mxcsr_ld(MS, DS)
#endif /* RT_SIMD_CODE */

/* ---------------------------------   X86   -------------------------------- */

#elif (defined RT_X86) /* original legacy target, supports only 8 registers */

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p2", "#p1
#define ASM_OP3(op, p1, p2, p3) #op"  "#p3", "#p2", "#p1

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMPTY                   ASM_BEG ASM_END /* endian-agnostic */
#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define EMITW(w)                ASM_BEG ASM_OP1(.long, w) ASM_END

#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(movl, %%eax, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(movl, lb, %%eax) ASM_END

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(leal, %%eax, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        EMITB(0x89)                                                         \
        MRM(0x00,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#ifndef RT_SIMD_FAST_FCTRL
#define RT_SIMD_FAST_FCTRL      (Q/2 != 0) /* if build includes wider targets */
#endif /* RT_SIMD_FAST_FCTRL */

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#ifndef RT_SIMD_FLUSH_ZERO
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
#endif /* RT_SIMD_FLUSH_ZERO */

/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RCP
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
#endif /* RT_SIMD_COMPAT_RCP */

/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RSQ
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
#endif /* RT_SIMD_COMPAT_RSQ */

/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMA
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
#endif /* RT_SIMD_COMPAT_FMA */

/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMS
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
#endif /* RT_SIMD_COMPAT_FMS */

/* RT_SIMD_COMPAT_FMR when enabled changes the default behavior
 * of fm*ps_** instruction fallbacks to honour rounding mode */
#ifndef RT_SIMD_COMPAT_FMR
#define RT_SIMD_COMPAT_FMR      RT_SIMD_COMPAT_FMR_MASTER
#endif /* RT_SIMD_COMPAT_FMR */

/* RT_BASE_COMPAT_BMI when enabled changes the default behavior
 * of some bit-manipulation instructions to use BMI variants */
#ifdef  RT_SIMD_CODE
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 2 /* enable BMI1+BMI2 when SIMD target is chosen */
#if   (RT_256X1 == 1) && (RT_SIMD == 256)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#elif (RT_128X1 < 32) && (RT_SIMD == 128)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#endif /* RT_SIMD: 256, 128 */
#endif /* RT_SIMD_CODE */

#if   (RT_2K8X1 != 0) && (RT_SIMD == 2048)
#error "x86:386 doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_1K4X2 != 0) && (RT_SIMD == 2048)
#error "x86:386 doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_512X4 != 0) && (RT_SIMD == 2048)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#elif (RT_1K4X1 != 0) && (RT_SIMD == 1024)
#error "x86:386 doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_512X2 != 0) && (RT_SIMD == 1024)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#elif (RT_256X4 != 0) && (RT_SIMD == 1024)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#elif (RT_512X1 >= 1) && (RT_SIMD == 512) && (RT_REGS <= 8)
#include "rtarch_x86_512x1v2.h"
#elif (RT_256X2 != 0) && (RT_SIMD == 512)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#elif (RT_128X4 != 0) && (RT_SIMD == 512)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#elif (RT_256X1 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 8)
#include "rtarch_x86_256x1v2.h"
#elif (RT_128X2 != 0) && (RT_SIMD == 256)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#elif (RT_128X1 >= 8) && (RT_SIMD == 128) && (RT_REGS <= 8)
#include "rtarch_x86_128x1v8.h"
#elif (RT_128X1 >= 1) && (RT_SIMD == 128) && (RT_REGS <= 8)
#include "rtarch_x86_128x1v4.h"
#else  /* report an error if header file is not selected */
#error "couldn't select appropriate SIMD target, check build flags"
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The SIMD unit is set to operate in its default mode (non-IEEE on ARMv7).
 */

#if RT_SIMD_FLUSH_ZERO == 0
#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(3*4), IH(0x7F80))                          \
        movwx_mi(Mebp, inf_FCTRL(2*4), IH(0x5F80))                          \
        movwx_mi(Mebp, inf_FCTRL(1*4), IH(0x3F80))                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */
#else /* RT_SIMD_FLUSH_ZERO */

#define ASM_ENTER(__Info__) ASM_ENTER_F(__Info__)

#define ASM_LEAVE(__Info__) ASM_LEAVE_F(__Info__)

#endif /* RT_SIMD_FLUSH_ZERO */

/*
 * The ASM_ENTER_F/ASM_LEAVE_F versions share the traits of the original ones,
 * except that they put the SIMD unit into slightly faster non-IEEE mode,
 * where denormal results from floating point operations are flushed to zero.
 * This mode is closely compatible with ARMv7, which lacks full IEEE support.
 */

#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)                                               \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))                                      \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    asm volatile                                                            \
    (                                                                       \
        movlb_st(%[Reax_])                                                  \
        movlb_ld(%[Info_])                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(3*4), IH(0xFF80))                          \
        movwx_mi(Mebp, inf_FCTRL(2*4), IH(0xDF80))                          \
        movwx_mi(Mebp, inf_FCTRL(1*4), IH(0xBF80))                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)                                               \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))                                      \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(%[Reax_])                                                  \
        : [Reax_] "+r" (__Reax__)                                           \
        : [Info_]  "r" ((rt_word)__Info__)                                  \
        : "cc",  "memory"                                                   \
    );                                                                      \
}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef RT_SIMD_CODE
#define sregs_sa()
#define sregs_la()
#define mxcsr_ld(MS, DS)
#endif /* RT_SIMD_CODE */

#endif /* RT_ARM, RT_A32/A64, RT_M32/M64, RT_P32/P64, RT_X32/X64, RT_X86 */

/*******************************   WIN32, MSVC   ******************************/

#elif (defined RT_WIN32)

/* ---------------------------------   X86   -------------------------------- */

#if   (defined RT_X86) /* original legacy target, supports only 8 registers */

#define ASM_OP0(op)             op
#define ASM_OP1(op, p1)         op  p1
#define ASM_OP2(op, p1, p2)     op  p1, p2
#define ASM_OP3(op, p1, p2, p3) op  p1, p2, p3

#define ASM_BEG /*internal*/    __asm {
#define ASM_END /*internal*/    }

#define EMPTY                   ASM_BEG ASM_END /* endian-little */
#define EMITB(b)                ASM_BEG ASM_OP1(_emit, b) ASM_END
#define EMITW(w)                EMITB((w) >> 0x00 & 0xFF)                   \
                                EMITB((w) >> 0x08 & 0xFF)                   \
                                EMITB((w) >> 0x10 & 0xFF)                   \
                                EMITB((w) >> 0x18 & 0xFF)

#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, eax, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, lb, eax) ASM_END

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(lea, eax, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        EMITB(0x89)                                                         \
        MRM(0x00,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#ifndef RT_SIMD_FAST_FCTRL
#define RT_SIMD_FAST_FCTRL      (Q/2 != 0) /* if build includes wider targets */
#endif /* RT_SIMD_FAST_FCTRL */

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#ifndef RT_SIMD_FLUSH_ZERO
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
#endif /* RT_SIMD_FLUSH_ZERO */

/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RCP
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
#endif /* RT_SIMD_COMPAT_RCP */

/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_RSQ
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
#endif /* RT_SIMD_COMPAT_RSQ */

/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMA
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
#endif /* RT_SIMD_COMPAT_FMA */

/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#ifndef RT_SIMD_COMPAT_FMS
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
#endif /* RT_SIMD_COMPAT_FMS */

/* RT_SIMD_COMPAT_FMR when enabled changes the default behavior
 * of fm*ps_** instruction fallbacks to honour rounding mode */
#ifndef RT_SIMD_COMPAT_FMR
#define RT_SIMD_COMPAT_FMR      RT_SIMD_COMPAT_FMR_MASTER
#endif /* RT_SIMD_COMPAT_FMR */

/* RT_BASE_COMPAT_BMI when enabled changes the default behavior
 * of some bit-manipulation instructions to use BMI variants */
#ifdef  RT_SIMD_CODE
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 2 /* enable BMI1+BMI2 when SIMD target is chosen */
#if   (RT_256X1 == 1) && (RT_SIMD == 256)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#elif (RT_128X1 < 32) && (RT_SIMD == 128)
#undef  RT_BASE_COMPAT_BMI
#define RT_BASE_COMPAT_BMI 0 /* no BMI1+BMI2 for SIMD targets prior to AVX2 */
#endif /* RT_SIMD: 256, 128 */
#endif /* RT_SIMD_CODE */

#if   (RT_2K8X1 != 0) && (RT_SIMD == 2048)
#error "x86:386 doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_1K4X2 != 0) && (RT_SIMD == 2048)
#error "x86:386 doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_512X4 != 0) && (RT_SIMD == 2048)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#elif (RT_1K4X1 != 0) && (RT_SIMD == 1024)
#error "x86:386 doesn't support SIMD wider than 512-bit, check build flags"
#elif (RT_512X2 != 0) && (RT_SIMD == 1024)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#elif (RT_256X4 != 0) && (RT_SIMD == 1024)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#elif (RT_512X1 >= 1) && (RT_SIMD == 512) && (RT_REGS <= 8)
#include "rtarch_x86_512x1v2.h"
#elif (RT_256X2 != 0) && (RT_SIMD == 512)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#elif (RT_128X4 != 0) && (RT_SIMD == 512)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#elif (RT_256X1 >= 1) && (RT_SIMD == 256) && (RT_REGS <= 8)
#include "rtarch_x86_256x1v2.h"
#elif (RT_128X2 != 0) && (RT_SIMD == 256)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#elif (RT_128X1 >= 8) && (RT_SIMD == 128) && (RT_REGS <= 8)
#include "rtarch_x86_128x1v8.h"
#elif (RT_128X1 >= 1) && (RT_SIMD == 128) && (RT_REGS <= 8)
#include "rtarch_x86_128x1v4.h"
#else  /* report an error if header file is not selected */
#error "couldn't select appropriate SIMD target, check build flags"
#endif /* RT_SIMD: 2048, 1024, 512, 256, 128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a sizeable portion of registers onto/from
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, in which case the ASM overhead is minimized.
 * The SIMD unit is set to operate in its default mode (non-IEEE on ARMv7).
 */

#if RT_SIMD_FLUSH_ZERO == 0
#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    __asm                                                                   \
    {                                                                       \
        movlb_st(__Reax__)                                                  \
        movlb_ld(__Info__)                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(__Reax__)                                                  \
    }                                                                       \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__)                                                 \
{                                                                           \
    rt_word __Reax__;                                                       \
    __asm                                                                   \
    {                                                                       \
        movlb_st(__Reax__)                                                  \
        movlb_ld(__Info__)                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(3*4), IH(0x7F80))                          \
        movwx_mi(Mebp, inf_FCTRL(2*4), IH(0x5F80))                          \
        movwx_mi(Mebp, inf_FCTRL(1*4), IH(0x3F80))                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)                                                 \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(__Reax__)                                                  \
    }                                                                       \
}

#endif /* RT_SIMD_FAST_FCTRL */
#else /* RT_SIMD_FLUSH_ZERO */

#define ASM_ENTER(__Info__) ASM_ENTER_F(__Info__)

#define ASM_LEAVE(__Info__) ASM_LEAVE_F(__Info__)

#endif /* RT_SIMD_FLUSH_ZERO */

/*
 * The ASM_ENTER_F/ASM_LEAVE_F versions share the traits of the original ones,
 * except that they put the SIMD unit into slightly faster non-IEEE mode,
 * where denormal results from floating point operations are flushed to zero.
 * This mode is closely compatible with ARMv7, which lacks full IEEE support.
 */

#if RT_SIMD_FAST_FCTRL == 0

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    __asm                                                                   \
    {                                                                       \
        movlb_st(__Reax__)                                                  \
        movlb_ld(__Info__)                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)                                               \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))                                      \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(__Reax__)                                                  \
    }                                                                       \
}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__)                                               \
{                                                                           \
    rt_word __Reax__;                                                       \
    __asm                                                                   \
    {                                                                       \
        movlb_st(__Reax__)                                                  \
        movlb_ld(__Info__)                                                  \
        stack_sa()                                                          \
        movxx_rr(Rebp, Reax)                                                \
        sregs_sa()                                                          \
        movwx_mi(Mebp, inf_FCTRL(3*4), IH(0xFF80))                          \
        movwx_mi(Mebp, inf_FCTRL(2*4), IH(0xDF80))                          \
        movwx_mi(Mebp, inf_FCTRL(1*4), IH(0xBF80))                          \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)                                               \
        movwx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))                          \
        mxcsr_ld(Mebp, inf_FCTRL(0*4))                                      \
        sregs_la()                                                          \
        stack_la()                                                          \
        movlb_ld(__Reax__)                                                  \
    }                                                                       \
}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef RT_SIMD_CODE
#define sregs_sa()
#define sregs_la()
#define mxcsr_ld(MS, DS)
#endif /* RT_SIMD_CODE */

#endif /* RT_X86 */

#endif /* OS, COMPILER, ARCH */

#endif /* RT_RTARCH_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

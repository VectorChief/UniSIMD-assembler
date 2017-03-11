/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtzero.h: Clean up of internal preprocessor short names.
 * Can be used to avoid collisions with system headers (mostly windows.h).
 */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * Alphabetical view of current/future instruction namespaces:
 *
 * cmda*_** - SIMD-data args, SIMD ISA (data-element is 16-bit, packed-256-bit)
 * cmdb*_** - byte-size args, BASE ISA (displacement/alignment may differ)
 * cmdc*_** - SIMD-data args, SIMD ISA (data-element is 32-bit, packed-256-bit)
 * cmdd*_** - SIMD-data args, SIMD ISA (data-element is 64-bit, packed-256-bit)
 * cmde*_** - extd-size args, extd ISA (for 80-bit extended double, x87)
 * cmdf*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-256-bit)
 * cmdg*_** - SIMD-data args, SIMD ISA (data-element is 16-bit, packed-128-bit)
 * cmdh*_** - half-size args, BASE ISA (displacement/alignment may differ)
 * cmdi*_** - SIMD-data args, SIMD ISA (data-element is 32-bit, packed-128-bit)
 * cmdj*_** - SIMD-data args, SIMD ISA (data-element is 64-bit, packed-128-bit)
 * cmdk*_** - king-kong args, BASE ISA (for 128-bit BASE subset, RISC-V)
 * cmdl*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-128-bit)
 * cmdm*_** - SIMD-data args, SIMD ISA (packed fp16/int subset, half-precision)
 * cmdn*_** - SIMD-elem args, SIMD ISA (scalar fp16/int subset, half-precision)
 * cmdo*_** - SIMD-data args, SIMD ISA (data-element is 32-bit, packed)
 * cmdp*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed)
 * cmdq*_** - SIMD-data args, SIMD ISA (data-element is 64-bit, packed)
 * cmdr*_** - SIMD-elem args, SIMD ISA (data-element is 32-bit, scalar)
 * cmds*_** - SIMD-elem args, SIMD ISA (32/64-bit configurable, scalar)
 * cmdt*_** - SIMD-elem args, SIMD ISA (data-element is 64-bit, scalar)
 * cmdu*_** - SIMD-data args, SIMD ISA (packed f128/int subset, quad-precision)
 * cmdv*_** - SIMD-elem args, SIMD ISA (scalar f128/int subset, quad-precision)
 * cmdw*_** - word-size args, BASE ISA (data-element is always fixed at 32-bit)
 * cmdx*_** - addr-size args, BASE ISA (32/64-bit configurable with RT_ADDRESS)
 * cmdy*_** - elem-size args, BASE ISA (32/64-bit configurable with RT_ELEMENT)
 * cmdz*_** - full-size args, BASE ISA (data-element is always fixed at 64-bit)
 *
 * More detailed description of the above is given in rtarch.h.
 */

/*
 * Configurable BASE/SIMD subsets (cmdx*, cmdy*, cmdp*) are defined in rtbase.h.
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
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 *
 * Setting-flags instruction naming scheme may change again in the future for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and Power use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
 *
 * The cmdp*_** (rtbase.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtbase.h as well.
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing of 256/512-bit.
 *
 * Working with sub-word BASE elements (byte, half) is reserved for future use.
 * However, current displacement types may not work due to natural alignment.
 * Signed/unsigned types can be supported orthogonally in cmd*n_**, cmd*x_**.
 * Working with sub-word SIMD elements (byte, half) has not been investigated.
 * However, as current major ISAs lack the ability to do sub-word fp-compute,
 * these corresponding subsets cannot be viewed as valid targets for SPMD.
 *
 * Scalar SIMD subset, horizontal SIMD reductions, constructive 3/4-op syntax
 * (potentially with zeroing/merging predicates) are being considered as future
 * extensions to current 2-op (dest-as-1st-src) SPMD-driven vertical SIMD ISA.
 */

#undef Q /* short name for SIMD-quads in structs (number of 128-bit chunks) */

#undef N /* short name for SIMD-width in structs (with rt_fp16 SIMD-fields) */
#undef R /* short name for SIMD-width in structs (with rt_fp32 SIMD-fields) */
#undef S /* short name for SIMD-width in structs (with rt_real SIMD-fields) */
#undef T /* short name for SIMD-width in structs (with rt_fp64 SIMD-fields) */

#undef W /* triplet pass-through wrapper */

#undef P /* short name for RT_POINTER/32 */
#undef A /* short name for RT_ADDRESS/32 */
#undef L /* short name for RT_ELEMENT/32 */

#undef B /* short name for RT_ENDIAN*(2-1)*4 */
#undef C /* short name for RT_ENDIAN*(2-A)*4 */
#undef D /* short name for RT_ENDIAN*(P-1)*4 */
#undef E /* short name for RT_ENDIAN*(P-A)*4 */
#undef F /* short name for RT_ENDIAN*(A-1)*4 */
#undef G /* short name for RT_ENDIAN*(P-2)*4 */
#undef H /* short name for RT_ENDIAN*(L-1)*4 */
#undef I /* short name for RT_ENDIAN*(2-L)*4 */

#undef Xmm0 /* external name for SIMD register */
#undef Xmm1 /* external name for SIMD register */
#undef Xmm2 /* external name for SIMD register */
#undef Xmm3 /* external name for SIMD register */
#undef Xmm4 /* external name for SIMD register */
#undef Xmm5 /* external name for SIMD register */
#undef Xmm6 /* external name for SIMD register */
#undef Xmm7 /* external name for SIMD register */
#undef Xmm8 /* external name for SIMD register */
#undef Xmm9 /* external name for SIMD register */
#undef XmmA /* external name for SIMD register */
#undef XmmB /* external name for SIMD register */
#undef XmmC /* external name for SIMD register */
#undef XmmD /* external name for SIMD register */
#undef XmmE /* external name for SIMD register, may be reserved in some cases */
#undef XmmF /* external name for SIMD register, may be reserved in some cases */

/* The last two SIMD registers can be reserved by the assembler when building
 * RISC targets with SIMD wider than natively supported 128-bit, in which case
 * they will be occupied by temporary data. Two hidden registers may also come
 * in handy when implementing elaborate register-spill techniques in the future
 * for current targets with less native registers than architecturally exposed.
 *
 * It should be possible to reserve only 1 SIMD register (XmmF) to achieve the
 * goals above (totalling 15 regs) at the cost of extra loads in certain ops. */

/******************************************************************************/
/*****************   SIMD FLAGS BACKWARD COMPATIBILITY LAYER   ****************/
/******************************************************************************/

#define RT_SIMD_COMPAT_256_MASTER       2 /* for 256-bit SSE(2,4) - 2,4 (x64) */
#define RT_SIMD_COMPAT_512_MASTER       1 /* for 512-bit AVX(1,2) - 1,2 (x64) */
#define RT_SIMD_COMPAT_2K8_MASTER       1 /* for 2K8-bit AVX3.1/2 - 1,2 (x64) */

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

#if (RT_2K8 != 0)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#endif
#if (RT_1K4 != 0)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#endif
#if (RT_512 &  8)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#endif
#if (RT_256 &  8)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#endif
#if (RT_512 != 0) && (RT_512X1 == 0)
#define RT_512X1  RT_512
#endif
#if (RT_256 != 0) && (RT_256X1 == 0)
#define RT_256X1  RT_256
#endif
#if (RT_128 != 0) && (RT_128X1 == 0)
#define RT_128X1  RT_128
#endif /* RT_2K8, RT_1K4, RT_512, RT_256, RT_128 */

/* ------------------------------   X32, X64   ------------------------------ */

#elif defined (RT_X32) || defined (RT_X64)

/* RT_SIMD_COMPAT_256 distinguishes between 256-bit SSE2 & SSE4
 * when RT_256=8 SIMD backend is present among build targets */
#ifndef RT_SIMD_COMPAT_256
#define RT_SIMD_COMPAT_256      RT_SIMD_COMPAT_256_MASTER
#endif /* RT_SIMD_COMPAT_256 */

/* RT_SIMD_COMPAT_512 distinguishes between 512-bit AVX1 & AVX2
 * when RT_512=8 SIMD backend is present among build targets */
#ifndef RT_SIMD_COMPAT_512
#define RT_SIMD_COMPAT_512      RT_SIMD_COMPAT_512_MASTER
#endif /* RT_SIMD_COMPAT_512 */

/* RT_SIMD_COMPAT_2K8 distinguishes between 2K8-bit AVX3.1 & .2
 * when RT_2K8=8 SIMD backend is present among build targets */
#ifndef RT_SIMD_COMPAT_2K8
#define RT_SIMD_COMPAT_2K8      RT_SIMD_COMPAT_2K8_MASTER
#endif /* RT_SIMD_COMPAT_2K8 */

#if (RT_2K8 &  8) && (RT_512X4 == 0)
#define RT_512X4  RT_SIMD_COMPAT_2K8
#endif
#if (RT_1K4 != 0) && (RT_512X2 == 0)
#define RT_512X2  RT_1K4
#endif
#if (RT_512 != 0) && (RT_512X1 == 0)
#define RT_512X1  ((RT_512) & 7)
#endif
#if (RT_512 &  8) && (RT_256X2 == 0)
#define RT_256X2  RT_SIMD_COMPAT_512
#endif
#if (RT_256 != 0) && (RT_256X1 == 0)
#define RT_256X1  ((RT_256) & 7)
#endif
#if (RT_256 &  8) && (RT_128X2 == 0)
#define RT_128X2  RT_SIMD_COMPAT_256
#endif
#if (RT_128 != 0) && (RT_128X1 == 0)
#define RT_128X1  RT_128
#endif /* RT_2K8, RT_1K4, RT_512, RT_256, RT_128 */

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)

#if (RT_2K8 != 0)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if (RT_1K4 != 0)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if (RT_512 != 0)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if (RT_256 != 0)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if (RT_128 != 0) && (RT_128X1 == 0)
#define RT_128X1  RT_128
#endif /* RT_2K8, RT_1K4, RT_512, RT_256, RT_128 */

/* ------------------------------   A32, A64   ------------------------------ */

#elif defined (RT_A32) || defined (RT_A64)

#if (RT_2K8 != 0)
#error "AArch64 doesn't support SIMD wider than 256-bit, check build flags"
#endif
#if (RT_1K4 != 0)
#error "AArch64 doesn't support SIMD wider than 256-bit, check build flags"
#endif
#if (RT_512 != 0)
#error "AArch64 doesn't support SIMD wider than 256-bit, check build flags"
#endif
#if (RT_256 != 0) && (RT_128X2 == 0)
#define RT_128X2  RT_256
#endif
#if (RT_128 != 0) && (RT_128X1 == 0)
#define RT_128X1  RT_128
#endif /* RT_2K8, RT_1K4, RT_512, RT_256, RT_128 */

/* ------------------------------   M32, M64   ------------------------------ */

#elif defined (RT_M32) || defined (RT_M64)

#if (RT_2K8 != 0)
#error "mipsMSA doesn't support SIMD wider than 256-bit, check build flags"
#endif
#if (RT_1K4 != 0)
#error "mipsMSA doesn't support SIMD wider than 256-bit, check build flags"
#endif
#if (RT_512 != 0)
#error "mipsMSA doesn't support SIMD wider than 256-bit, check build flags"
#endif
#if (RT_256 != 0) && (RT_128X2 == 0)
#define RT_128X2  RT_256
#endif
#if (RT_128 != 0) && (RT_128X1 == 0)
#define RT_128X1  RT_128
#endif /* RT_2K8, RT_1K4, RT_512, RT_256, RT_128 */

/* ------------------------------   P32, P64   ------------------------------ */

#elif defined (RT_P32) || defined (RT_P64)

#if (RT_2K8 != 0)
#error "PowerPC doesn't support SIMD wider than 512-bit, check build flags"
#endif
#if (RT_1K4 != 0)
#error "PowerPC doesn't support SIMD wider than 512-bit, check build flags"
#endif
#if (RT_512 != 0) && (RT_128X4 == 0)
#define RT_128X4  ((RT_512)<<1)
#endif
#if (RT_256 != 0) && (RT_128X2 == 0)
#define RT_128X2  (((RT_256)/8 & 1) | ((RT_256)*2 & 6))
#endif
#if (RT_128 != 0) && (RT_128X1 == 0)
#define RT_128X1  RT_128
#endif /* RT_2K8, RT_1K4, RT_512, RT_256, RT_128 */

#endif /* RT_X86, RT_X32/X64, RT_ARM, RT_A32/A64, RT_M32/M64, RT_P32/P64 */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

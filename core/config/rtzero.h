/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
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
 * cmda*_** - SIMD-data args, SIMD ISA (data element is 16-bit, packed-256-bit)
 * cmdb*_** - byte-size args, BASE ISA (displacement/alignment may differ)
 * cmdc*_** - SIMD-data args, SIMD ISA (data element is 32-bit, packed-256-bit)
 * cmdd*_** - SIMD-data args, SIMD ISA (data element is 64-bit, packed-256-bit)
 * cmde*_** - extd-size args, extd ISA (for 80-bit extended double, x87)
 * cmdf*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-256-bit)
 * cmdg*_** - SIMD-data args, SIMD ISA (data element is 16-bit, packed-128-bit)
 * cmdh*_** - half-size args, BASE ISA (displacement/alignment may differ)
 * cmdi*_** - SIMD-data args, SIMD ISA (data element is 32-bit, packed-128-bit)
 * cmdj*_** - SIMD-data args, SIMD ISA (data element is 64-bit, packed-128-bit)
 * cmdk*_** - king-kong args, BASE ISA (for 128-bit BASE subset, RISC-V)
 * cmdl*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-128-bit)
 * cmdm*_** - SIMD-data args, SIMD ISA (packed fp16/int subset, half-precision)
 * cmdn*_** - SIMD-elem args, ELEM ISA (scalar fp16/int subset, half-precision)
 * cmdo*_** - SIMD-data args, SIMD ISA (data element is 32-bit, packed)
 * cmdp*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed)
 * cmdq*_** - SIMD-data args, SIMD ISA (data element is 64-bit, packed)
 * cmdr*_** - SIMD-elem args, ELEM ISA (data element is 32-bit, scalar)
 * cmds*_** - SIMD-elem args, ELEM ISA (32/64-bit configurable, scalar)
 * cmdt*_** - SIMD-elem args, ELEM ISA (data element is 64-bit, scalar)
 * cmdu*_** - SIMD-data args, SIMD ISA (packed f128/int subset, quad-precision)
 * cmdv*_** - SIMD-elem args, ELEM ISA (scalar f128/int subset, quad-precision)
 * cmdw*_** - word-size args, BASE ISA (data element is always fixed at 32-bit)
 * cmdx*_** - addr-size args, BASE ISA (32/64-bit configurable with RT_ADDRESS)
 * cmdy*_** - elem-size args, BASE ISA (32/64-bit configurable with RT_ELEMENT)
 * cmdz*_** - full-size args, BASE ISA (data element is always fixed at 64-bit)
 *
 * More detailed description of the above is given in rtarch.h.
 */

/*
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

#undef Q /* short name for SIMD quads in structs (number of 128-bit chunks) */
#undef O /* short name for SIMD quads scale-factor in address-displacements */

#undef N /* short name for SIMD width in structs (with rt_fp16 SIMD-fields) */
#undef R /* short name for SIMD width in structs (with rt_fp32 SIMD-fields) */
#undef S /* short name for SIMD width in structs (with rt_real SIMD-fields) */
#undef T /* short name for SIMD width in structs (with rt_fp64 SIMD-fields) */

#undef W /* triplet pass-through wrapper */

#undef P /* short name for RT_POINTER/32 */
#undef A /* short name for RT_ADDRESS/32 */
#undef L /* short name for RT_ELEMENT/32 */

#undef M /* true-condition sign llvm/gas */

#undef B /* short name for RT_ENDIAN*(2-1)*4 */
#undef C /* short name for RT_ENDIAN*(2-A)*4 */
#undef D /* short name for RT_ENDIAN*(P-1)*4 */
#undef E /* short name for RT_ENDIAN*(P-A)*4 */
#undef F /* short name for RT_ENDIAN*(A-1)*4 */
#undef G /* short name for RT_ENDIAN*(P-2)*4 */
#undef H /* short name for RT_ENDIAN*(L-1)*4 */
#undef I /* short name for RT_ENDIAN*(2-L)*4 */

#undef IC /* immediate type:     0x0000007F */
#undef IB /* immediate type:     0x000000FF */
#undef IM /* immediate type:     0x00000FFF */
#undef IG /* immediate type:     0x00007FFF */
#undef IH /* immediate type:     0x0000FFFF */
#undef IV /* immediate type:     0x7FFFFFFF */
#undef IW /* immediate type:     0xFFFFFFFF, only for cmdw*_** subset */

#undef DP /* displacement type:  0x00000FFF */
#undef DE /* displacement type:  0x00001FFF */
#undef DF /* displacement type:  0x00003FFF */
#undef DG /* displacement type:  0x00007FFF */
#undef DH /* displacement type:  0x0000FFFF */
#undef DV /* displacement type:  0x7FFFFFFF */
#undef PLAIN     /* plain type:  0x00000000, only for Oeax addressing */

#undef Oeax /* external name for BASE-plain addressing */

#undef Mebx /* external name for BASE+displ addressing */
#undef Mecx /* external name for BASE+displ addressing */
#undef Medx /* external name for BASE+displ addressing */
#undef Mebp /* external name for BASE+displ addressing */
#undef Mesi /* external name for BASE+displ addressing */
#undef Medi /* external name for BASE+displ addressing */
#undef Meg8 /* external name for BASE+displ addressing */
#undef Meg9 /* external name for BASE+displ addressing */
#undef MegA /* external name for BASE+displ addressing */
#undef MegB /* external name for BASE+displ addressing */
#undef MegC /* external name for BASE+displ addressing */
#undef MegD /* external name for BASE+displ addressing */
#undef MegE /* external name for BASE+displ addressing */

#undef Iebx /* external name for BASE+index+displ addressing */
#undef Iecx /* external name for BASE+index+displ addressing */
#undef Iedx /* external name for BASE+index+displ addressing */
#undef Iebp /* external name for BASE+index+displ addressing */
#undef Iesi /* external name for BASE+index+displ addressing */
#undef Iedi /* external name for BASE+index+displ addressing */
#undef Ieg8 /* external name for BASE+index+displ addressing */
#undef Ieg9 /* external name for BASE+index+displ addressing */
#undef IegA /* external name for BASE+index+displ addressing */
#undef IegB /* external name for BASE+index+displ addressing */
#undef IegC /* external name for BASE+index+displ addressing */
#undef IegD /* external name for BASE+index+displ addressing */
#undef IegE /* external name for BASE+index+displ addressing */

#undef Reax /* external name for BASE register */
#undef Rebx /* external name for BASE register */
#undef Recx /* external name for BASE register */
#undef Redx /* external name for BASE register */
#undef Rebp /* external name for BASE register, may be reserved in most cases */
#undef Resi /* external name for BASE register */
#undef Redi /* external name for BASE register */
#undef Reg8 /* external name for BASE register */
#undef Reg9 /* external name for BASE register */
#undef RegA /* external name for BASE register */
#undef RegB /* external name for BASE register */
#undef RegC /* external name for BASE register */
#undef RegD /* external name for BASE register */
#undef RegE /* external name for BASE register */

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
#undef XmmE /* external name for SIMD register */
#undef XmmF /* external name for SIMD register, may be reserved in some cases */

#undef XmmG /* external name for SIMD register */
#undef XmmH /* external name for SIMD register */
#undef XmmI /* external name for SIMD register */
#undef XmmJ /* external name for SIMD register */
#undef XmmK /* external name for SIMD register */
#undef XmmL /* external name for SIMD register */
#undef XmmM /* external name for SIMD register */
#undef XmmN /* external name for SIMD register */
#undef XmmO /* external name for SIMD register */
#undef XmmP /* external name for SIMD register */
#undef XmmQ /* external name for SIMD register */
#undef XmmR /* external name for SIMD register */
#undef XmmS /* external name for SIMD register */
#undef XmmT /* external name for SIMD register */
#undef XmmU /* external name for SIMD register, may be reserved in most cases */
#undef XmmV /* external name for SIMD register, may be reserved in most cases */

/******************************************************************************/
/*****************   SIMD FLAGS EXTENDED COMPATIBILITY LAYER   ****************/
/******************************************************************************/

/* Interpretation of a 32-bit SIMD-version field (ver) in rt_SIMD_INFO struct:
 *
 * |RT_128 |256_R8 |RT_256 |512_R8 |RT_512 |1K4_R8 |RT_1K4 |2K8_R8 | - SIMD-flag
 * |1 2 4 8|- - 4|*|1 2 4 8|1 2 -|*|1 2 4 8|1 2 -|*|1 2 4 8|1 2 -|*| - cur-value
 * |       16 32   |       |       |       |       |       |       | - ext-value
 * |0              |               |               |             31| - bit-order
 * |-o-o-o-i-o-o-p-|-o-o-o-i-o-o-p-|-o-o-o-i-o-o-p-|-o-o-o-i-o-o-p-| - SIMD-mask
 *                ^               ^               ^               ^
 *             128_RX          256_RX          512_RX          1K4_RX
 *
 * In current scheme: RT_128=4+8, RT_256=1+2, RT_512=1+2, RT_1K4=1+2 are 15-reg.
 * In current scheme: RT_128=1+2, RT_256=4+8, RT_512=4+8, RT_1K4=4+8 are 30-reg.
 *
 * New bits can be freed for future targets by using RT_REGS=16/32 in makefiles
 * resulting in 4 variants per width in total (both 15/30-regs then start at 1).
 *
 * Original RT_128, RT_256, RT_512, RT_1K4 flags expose 15/30-register targets.
 * New RT_256_R8, RT_512_R8, RT_1K4_R8, RT_2K8_R8 flags are 8-register targets.
 * New RT_128_RX, RT_256_RX, RT_512_RX, RT_1K4_RX flags are predicated targets.
 * NOTE: predicated targets can be implemented as extensions to existing ones.
 *
 * The original RT_*** targets have only 8 registers on legacy CPUs (x86/ARMv7).
 * The original RT_*** targets can be either native (x64) or composite (RISCs).
 * The *_R8 targets are always composed of register/instruction pairs or quads.
 * The *_RX targets have 30 registers + 6 masks (AVX-512/1K4 and ARM-SVE only).
 *
 * Some targets may have less registers than category-defined maximum (8,16,32).
 * The common minimum in each category is then defined with RT_SIMD_COMPAT_XMM:
 * 0 - (8,16,32); 1 - (8,15,30); 2 - (7,14,28) and determines a set of targets.
 */

#if   !(defined RT_2K8_R8)
#define         RT_2K8_R8   0
#endif
#if   !(defined RT_1K4)
#define         RT_1K4      0
#endif
#if   !(defined RT_1K4_R8)
#define         RT_1K4_R8   0
#endif
#if   !(defined RT_512)
#define         RT_512      0
#endif
#if   !(defined RT_512_R8)
#define         RT_512_R8   0
#endif
#if   !(defined RT_256)
#define         RT_256      0
#endif
#if   !(defined RT_256_R8)
#define         RT_256_R8   0
#endif
#if   !(defined RT_128)
#define         RT_128      0
#endif

/* ---------------------------------   ARM   -------------------------------- */

#if   (defined RT_ARM) /* original legacy target, supports only 8 registers */

#if   !(defined RT_512X4) && (RT_2K8_R8)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_512X2) && (RT_1K4 || RT_1K4_R8)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_512X1) && (RT_512)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_256X2) && (RT_512_R8)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_256X1) && (RT_256)
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_128X2) && (RT_256_R8)
#error "AArch32 doesn't support paired SIMD backends, check build flags"
#endif
#if   !(defined RT_128X1) && (RT_128)
#define         RT_128X1     (RT_128)
#endif /* RT_2K8_R8, RT_1K4/_R8, RT_512/_R8, RT_256/_R8, RT_128 */

/* ------------------------------   A32, A64   ------------------------------ */

#elif (defined RT_A32) || (defined RT_A64)

#if   !(defined RT_SVEX1) && (RT_2K8_R8) & 0xC
#define         RT_2K8X1     (RT_2K8_R8)
#define         RT_SVEX1     (RT_2K8_R8)
#endif
#if   !(defined RT_SVEX2) && (RT_2K8_R8) & 0x3
#define         RT_1K4X2     (RT_2K8_R8)
#define         RT_SVEX2     (RT_2K8_R8)
#endif
#if   !(defined RT_SVEX1) && (RT_1K4 |  RT_1K4_R8) & 0xC
#define         RT_1K4X1     (RT_1K4 |  RT_1K4_R8)
#define         RT_SVEX1     (RT_1K4 |  RT_1K4_R8)
#endif
#if   !(defined RT_SVEX2) && (RT_1K4 |  RT_1K4_R8) & 0x3
#define         RT_512X2     (RT_1K4 |  RT_1K4_R8)
#define         RT_SVEX2     (RT_1K4 |  RT_1K4_R8)
#endif
#if   !(defined RT_SVEX1) && (RT_512 |  RT_512_R8) & 0xC
#define         RT_512X1     (RT_512 |  RT_512_R8)
#define         RT_SVEX1     (RT_512 |  RT_512_R8)
#endif
#if   !(defined RT_SVEX2) && (RT_512 |  RT_512_R8) & 0x3
#define         RT_256X2     (RT_512 |  RT_512_R8)
#define         RT_SVEX2     (RT_512 |  RT_512_R8)
#endif
#if   !(defined RT_SVEX1) && (RT_256) & 0xC
#define         RT_256X1     (RT_256)
#define         RT_SVEX1     (RT_256)
#endif
#if   !(defined RT_128X2) && (RT_256) & 0x3
#define         RT_128X2     (RT_256)
#endif
#if   !(defined RT_128X2) && (RT_256_R8)
#define         RT_128X2     (RT_256_R8)
#endif
#if   !(defined RT_128X1) && (RT_128)
#define         RT_128X1     (RT_128)
#endif /* RT_2K8_R8, RT_1K4/_R8, RT_512/_R8, RT_256/_R8, RT_128 */

/* ------------------------------   M32, M64   ------------------------------ */

#elif (defined RT_M32) || (defined RT_M64)

#if   !(defined RT_512X4) && (RT_2K8_R8)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_512X2) && (RT_1K4 || RT_1K4_R8)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_512X1) && (RT_512)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_256X2) && (RT_512_R8)
#error "mipsMSA doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_128X2) && (RT_256 || RT_256_R8)
#define         RT_128X2     (RT_256 |  RT_256_R8)
#endif
#if   !(defined RT_128X1) && (RT_128)
#define         RT_128X1     (RT_128)
#endif /* RT_2K8_R8, RT_1K4/_R8, RT_512/_R8, RT_256/_R8, RT_128 */

/* ------------------------------   P32, P64   ------------------------------ */

#elif (defined RT_P32) || (defined RT_P64)

#if   !(defined RT_512X4) && (RT_2K8_R8)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_512X2) && (RT_1K4 || RT_1K4_R8)
#error "PowerPC doesn't support SIMD wider than 128-bit, check build flags"
#endif
#if   !(defined RT_128X4) && (RT_512 || RT_512_R8)
#define         RT_128X4     (RT_512 |  RT_512_R8)
#endif
#if   !(defined RT_128X2) && (RT_256 || RT_256_R8)
#define         RT_128X2     (RT_256 | (RT_256_R8)*4)
#endif
#if   !(defined RT_128X1) && (RT_128)
#define         RT_128X1     (RT_128)
#endif /* RT_2K8_R8, RT_1K4/_R8, RT_512/_R8, RT_256/_R8, RT_128 */

/* ------------------------------   X32, X64   ------------------------------ */

#elif (defined RT_X32) || (defined RT_X64)

#if   !(defined RT_512X4) && (RT_2K8_R8)
#define         RT_512X4     (RT_2K8_R8)
#endif
#if   !(defined RT_512X2) && (RT_1K4 || RT_1K4_R8)
#define         RT_512X2     (RT_1K4 |  RT_1K4_R8)
#endif
#if   !(defined RT_512X1) && (RT_512)
#define         RT_512X1     (RT_512)
#endif
#if   !(defined RT_256X2) && (RT_512_R8)
#define         RT_256X2     (RT_512_R8)
#endif
#if   !(defined RT_256X1) && (RT_256)
#define         RT_256X1     (RT_256)
#endif
#if   !(defined RT_128X2) && (RT_256_R8)
#define         RT_128X2     (RT_256_R8)
#endif
#if   !(defined RT_128X1) && (RT_128)
#define         RT_128X1     (RT_128)
#endif /* RT_2K8_R8, RT_1K4/_R8, RT_512/_R8, RT_256/_R8, RT_128 */

/* ---------------------------------   X86   -------------------------------- */

#elif (defined RT_X86) /* original legacy target, supports only 8 registers */

#if   !(defined RT_512X4) && (RT_2K8_R8)
#error "x86:386 doesn't support quaded SIMD backends, check build flags"
#endif
#if   !(defined RT_512X2) && (RT_1K4 || RT_1K4_R8)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#endif
#if   !(defined RT_512X1) && (RT_512)
#define         RT_512X1     (RT_512)
#endif
#if   !(defined RT_256X2) && (RT_512_R8)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#endif
#if   !(defined RT_256X1) && (RT_256)
#define         RT_256X1     (RT_256)
#endif
#if   !(defined RT_128X2) && (RT_256_R8)
#error "x86:386 doesn't support paired SIMD backends, check build flags"
#endif
#if   !(defined RT_128X1) && (RT_128)
#define         RT_128X1     (RT_128)
#endif /* RT_2K8_R8, RT_1K4/_R8, RT_512/_R8, RT_256/_R8, RT_128 */

#endif /* RT_ARM, RT_A32/A64, RT_M32/M64, RT_P32/P64, RT_X32/X64, RT_X86 */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_H
#define RT_RTARCH_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch.h: Main architecture selection file.
 *
 * Main file of the unified SIMD assembler framework
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API. Namespaces for current
 * and future instruction subsets are defined (reserved) in the next section.
 *
 * Definitions provided in this file are intended to hide the differences of
 * inline assembly implementations in various compilers and operating systems,
 * while definitions of particular instruction sets are delegated to the
 * corresponding companion files named rtarch_***.h for BASE instructions
 * and rtarch_***_***.h for SIMD instructions.
 *
 * Note that AArch32 mode of ARMv8 ISA is a part of legacy ARM target as it
 * only brings SIMD fp-convert with round parameter plus other minor tweaks,
 * while IEEE-compatible SIMD fp-arithmetic with full square root and divide
 * is exposed via (ILP32 ABI of) AArch64:ARMv8 ISA in (A32 and) A64 target(s).
 *
 * Preliminary naming scheme for legacy, current and potential future targets.
 *
 * Legacy 32-bit BASE
 *    and 32-bit SIMD targets:
 *
 *  - rtarch_arm.h         - AArch32:ARMv7 ISA, 16 BASE regs, 8 + temps used
 *  - rtarch_arm_128v*.h   - 32-bit elements, 16 SIMD regs, 8 + temps used
 *  - rtarch_x86.h         - x86 32-bit ISA, 8 BASE regs, 6 + esp, ebp used
 *  - rtarch_x86_128v*.h   - 32-bit elements, 8 SIMD regs, SSE 128-bit, 8 used
 *  - rtarch_x86_256v*.h   - 32-bit elements, 8 SIMD regs, AVX 256-bit, 8 used
 *  - rtarch_x86_512v*.h   - 32-bit elements, 8 SIMD regs, AVX 512-bit, 8 used
 *
 * Current 32/64-bit BASE
 * and 32/64-bit SIMD targets:
 *
 *  - rtarch_a32.h         - AArch64:ILP32 ABI, 32 BASE regs, 14 + temps used
 *  - rtarch_a64.h         - AArch64:ARMv8 ISA, 32 BASE regs, 14 + temps used
 *  - rtarch_a32_128v*.h   - 32-bit elements, 32 SIMD regs, NEON 128-bit, 16+1
 *  - rtarch_a64_128v*.h   - 64-bit elements, 32 SIMD regs, NEON 128-bit, 16+1
 *  - rtarch_a32_256v*.h   - 32-bit elements, 32 SIMD regs, pairs of 128-bit, 15
 *  - rtarch_a64_256v*.h   - 64-bit elements, 32 SIMD regs, pairs of 128-bit, 15
 *  - rtarch_m32.h         - MIPS32 r5/r6 ISA, 32 BASE regs, 14 + temps used
 *  - rtarch_m64.h         - MIPS64 r5/r6 ISA, 32 BASE regs, 14 + temps used
 *  - rtarch_m32_128v*.h   - 32-bit elements, 32 SIMD regs, MSA 128-bit, 16+4(2)
 *  - rtarch_m64_128v*.h   - 64-bit elements, 32 SIMD regs, MSA 128-bit, 16+4(2)
 *  - rtarch_m32_256v*.h   - 32-bit elements, 32 SIMD regs, pairs of 128-bit, 15
 *  - rtarch_m64_256v*.h   - 64-bit elements, 32 SIMD regs, pairs of 128-bit, 15
 *  - rtarch_p32.h         - Power 32-bit ISA, 32 BASE regs, 14 + temps used
 *  - rtarch_p64.h         - Power 64-bit ISA, 32 BASE regs, 14 + temps used
 *  - rtarch_p32_128v*.h   - 32-bit elements, 32 SIMD regs, VMX/VSX 128-bit, 16+
 *  - rtarch_p64_128v*.h   - 64-bit elements, 32 SIMD regs, VMX/VSX 128-bit, 16+
 *  - rtarch_p32_256v*.h   - 32-bit elements, 64 SIMD regs, pairs of 128-bit, 15
 *  - rtarch_p64_256v*.h   - 64-bit elements, 64 SIMD regs, pairs of 128-bit, 15
 *  - rtarch_p32_512v*.h   - 32-bit elements, 64 SIMD regs, quads of 128-bit, 15
 *  - rtarch_p64_512v*.h   - 64-bit elements, 64 SIMD regs, quads of 128-bit, 15
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 BASE regs, 14 + temps used
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 BASE regs, 14 + temps used
 *  - rtarch_x32_128v*.h   - 32-bit elements, 16 SIMD regs, SSE 128-bit, 16 used
 *  - rtarch_x64_128v*.h   - 64-bit elements, 16 SIMD regs, SSE 128-bit, 16 used
 *  - rtarch_x32_256v*.h   - 32-bit elements, 16 SIMD regs, AVX 256-bit, 16 used
 *  - rtarch_x64_256v*.h   - 64-bit elements, 16 SIMD regs, AVX 256-bit, 16 used
 *  - rtarch_x32_512v*.h   - 32-bit elements, 32 SIMD regs, AVX 512-bit, 16 used
 *  - rtarch_x64_512v*.h   - 64-bit elements, 32 SIMD regs, AVX 512-bit, 16 used
 *
 * Future 32-bit SIMD targets:
 *
 *  - rtarch_a32_128v*.h   - 32-bit elements, 32 SIMD regs, NEON 128-bit, 30
 *  - rtarch_a32_256v*.h   - 32-bit elements, 32 SIMD regs, SVE 256-bit, 30 used
 *  - rtarch_a32_512v*.h   - 32-bit elements, 32 SIMD regs, SVE 512-bit, 30 used
 *  - rtarch_m32_128v*.h   - 32-bit elements, 32 SIMD regs, MSA 128-bit, 30 used
 *  - rtarch_p32_128v*.h   - 32-bit elements, 64 SIMD regs, VSX 128-bit, 30 used
 *  - rtarch_p32_256v*.h   - 32-bit elements, 64 SIMD regs, pairs of 128-bit, 30
 *  - rtarch_x32_512v*.h   - 32-bit elements, 32 SIMD regs, AVX 512-bit, 30 used
 *
 * Future 64-bit SIMD targets:
 *
 *  - rtarch_a64_128v*.h   - 64-bit elements, 32 SIMD regs, NEON 128-bit, 30
 *  - rtarch_a64_256v*.h   - 64-bit elements, 32 SIMD regs, SVE 256-bit, 30 used
 *  - rtarch_a64_512v*.h   - 64-bit elements, 32 SIMD regs, SVE 512-bit, 30 used
 *  - rtarch_m64_128v*.h   - 64-bit elements, 32 SIMD regs, MSA 128-bit, 30 used
 *  - rtarch_p64_128v*.h   - 64-bit elements, 64 SIMD regs, VSX 128-bit, 30 used
 *  - rtarch_p64_256v*.h   - 64-bit elements, 64 SIMD regs, pairs of 128-bit, 30
 *  - rtarch_x64_512v*.h   - 64-bit elements, 32 SIMD regs, AVX 512-bit, 30 used
 *
 * not all registers in target descriptions are always exposed for apps to use
 * flags RT_BASE_REGS and RT_SIMD_REGS are available for rough differentiation
 * between register-file sizes with current values: legacy 8, 16, 32 (planned)
 * while top registers reservation is controlled via RT_SIMD_COMPAT_XMM option
 *
 * Preliminary naming scheme for extended BASE and SIMD register-files.
 *
 * Legacy 8 BASE and 8 SIMD registers:
 *  - Reax, Rebx, Recx, Redx, Resp, Rebp, Resi, Redi
 *  - Xmm0, Xmm1, Xmm2, Xmm3, Xmm4, Xmm5, Xmm6, Xmm7
 *
 * Current 16 BASE and 16 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegF
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmF
 *
 * Future 32 BASE and 32 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegV
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmV
 *
 * Although register names are fixed, register sizes are not and depend
 * on the chosen target (32/64-bit BASE and 128/256-bit SIMD are supported).
 * Base registers can be 32/64-bit wide, while their SIMD counterparts
 * depend on the architecture and SIMD version chosen for the target.
 * On 64-bit systems SIMD can be configured to work with 32/64-bit elements.
 * Fractional sub-registers don't have names and aren't architecturally
 * visible in the assembler in order to simplify SPMD programming model.
 */

/******************************************************************************/

/*
 * The following BASE instruction namespaces are defined for current use.
 *
 * cmdw*_** - word-size args, BASE ISA (data-element is always fixed at 32-bit)
 * cmdx*_** - addr-size args, BASE ISA (32/64-bit configurable with RT_ADDRESS)
 * cmdy*_** - elem-size args, BASE ISA (32/64-bit configurable with RT_ELEMENT)
 * cmdz*_** - full-size args, BASE ISA (data-element is always fixed at 64-bit)
 *
 * cmd*z_** - usage for setting-flags is implemented orthogonal to data-size
 *
 * The following SIMD instruction namespaces are defined for current use.
 *
 * cmdo*_** - SIMD-data args, SIMD ISA (always fixed at 32-bit, packed)
 * cmdp*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed)
 * cmdq*_** - SIMD-data args, SIMD ISA (always fixed at 64-bit, packed)
 *
 * packed SIMD instructions above are vector-length-agnostic: (Q * 128-bit)
 *
 * The following SIMD instruction namespaces are reserved for future use.
 *
 * cmdr*_** - SIMD-elem args, SIMD ISA (always fixed at 32-bit, scalar)
 * cmds*_** - SIMD-elem args, SIMD ISA (32/64-bit configurable, scalar)
 * cmdt*_** - SIMD-elem args, SIMD ISA (always fixed at 64-bit, scalar)
 *
 * cmdg*_** - SIMD-data args, SIMD ISA (always fixed at 16-bit, packed-128-bit)
 * cmdi*_** - SIMD-data args, SIMD ISA (always fixed at 32-bit, packed-128-bit)
 * cmdj*_** - SIMD-data args, SIMD ISA (always fixed at 64-bit, packed-128-bit)
 * cmdl*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-128-bit)
 *
 * cmda*_** - SIMD-data args, SIMD ISA (always fixed at 16-bit, packed-256-bit)
 * cmdc*_** - SIMD-data args, SIMD ISA (always fixed at 32-bit, packed-256-bit)
 * cmdd*_** - SIMD-data args, SIMD ISA (always fixed at 64-bit, packed-256-bit)
 * cmdf*_** - SIMD-data args, SIMD ISA (32/64-bit configurable, packed-256-bit)
 *
 * fixed 256-bit ops can be done as pairs with 2*15 128-bit regs on modern RISCs
 * fixed 256-bit ops can be done as pairs with 2*30 128-bit regs on modern Power
 * while 512-bit ops can be done as quads with 4*15 128-bit regs on modern Power
 * potentially saving extra address-calculation step using indexed-load on Power
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
 * The following BASE instruction namespaces are planned for potential future.
 *
 * cmdb*_** - byte-size args, BASE ISA (displacement/alignment may differ)
 * cmdh*_** - half-size args, BASE ISA (displacement/alignment may differ)
 *
 * cmde*_** - extd-size args, extd ISA (for 80-bit extended double, x87)
 * cmdk*_** - king-kong args, BASE ISA (for 128-bit BASE subset, RISC-V)
 *
 * The following SIMD instruction namespaces are planned for potential future.
 *
 * cmdm*_** - SIMD-data args, SIMD ISA (packed fp16/int subset, half-precision)
 * cmdn*_** - SIMD-elem args, SIMD ISA (scalar fp16/int subset, half-precision)
 *
 * cmdu*_** - SIMD-data args, SIMD ISA (packed f128/int subset, quad-precision)
 * cmdv*_** - SIMD-elem args, SIMD ISA (scalar f128/int subset, quad-precision)
 *
 * cmdpb_** - SIMD-data args, SIMD ISA (packed byte-int subset)
 * cmdph_** - SIMD-data args, SIMD ISA (packed half-int subset)
 *
 * packed SIMD instructions above are vector-length-agnostic: (Q * 128-bit)
 *
 * Alphabetical view of current/future instruction namespaces is in rtzero.h.
 * Adjustable BASE/SIMD subsets (cmdx*, cmdy*, cmdp*) are defined in rtbase.h.
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
 * to produce consistent results usable in 64-bit subset across all targets.
 * Also registers written with 64-bit ops aren't always compatible with 32-bit,
 * as m64 requires the upper half to be all 0s or all 1s for m32 arithmetic.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subset,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (32-bit, 64-bit or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 *
 * Setting-flags instructions' naming scheme may change again in the future for
 * better orthogonality with operands size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and Power use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
 *
 * Working with sub-word BASE elements (byte, half) is reserved for future use.
 * However, current displacement types may not work due to natural alignment.
 * Signed/unsigned types can be supported orthogonally in cmd*n_**, cmd*x_**.
 * Working with sub-word SIMD elements (byte, half) has not been investigated.
 * However, as current major ISAs lack the ability to do sub-word fp-compute,
 * these corresponding subsets cannot be considered valid targets for SPMD.
 *
 * Scalar SIMD subset, horizontal SIMD reductions, constructive 3/4-op syntax
 * (potentially with zeroing/merging predicates) are being considered as future
 * extensions to current 2-op (dest-as-1st-src) SPMD-driven vertical SIMD ISA.
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
#define RT_SIMD_COMPAT_128_MASTER       1 /* for 128-bit AVX(1,2) - 1,2 (x86) */
#define RT_SIMD_COMPAT_FMR_MASTER       0 /* for fm*ps_** rounding mode (x86) */
#define RT_SIMD_FLUSH_ZERO_MASTER       0 /* optional on MIPS and Power */

#include "rtzero.h"

/*
 * SIMD quad-factor (number of 128-bit chunks) for chosen SIMD target.
 * Short name Q represents maximal quad-factor for given build config.
 * RT_SIMD_QUADS and Q may differ for builds with runtime SIMD target
 * selection in backend's ASM code sections, Q is used in SIMD structs.
 */
#define RT_SIMD_QUADS       (RT_SIMD_WIDTH32/4)

/*
 * SIMD width (number of scalar elements) for given SIMD element size.
 * Short names R, S, T represent maximal width for given build config.
 * RT_SIMD_WIDTH and S may differ for builds with runtime SIMD target
 * selection in backend's ASM code sections, S is used in SIMD structs.
 */
#define N   (Q*8)       /* for cmdm*_** SIMD-subset, rt_fp16 SIMD-fields */
#define R   (Q*4)       /* for cmdo*_** SIMD-subset, rt_fp32 SIMD-fields */
#define S   (Q*4/L)     /* for cmdp*_** SIMD-subset, rt_real SIMD-fields */
#define T   (Q*2)       /* for cmdq*_** SIMD-subset, rt_fp64 SIMD-fields */

/*
 * Short names for pointer, address and SIMD element sizes (in 32-bit chunks).
 */
#define P   (RT_POINTER/32)         /* short name for RT_POINTER/32 */
#define A   (RT_ADDRESS/32)         /* short name for RT_ADDRESS/32 */
#define L   (RT_ELEMENT/32)         /* short name for RT_ELEMENT/32 */

/*
 * Offset corrections for endianness (used in backend's structs and BASE ISA).
 */
#define B   (RT_ENDIAN*(2-1)*4)     /* for cmdw*_** working on 64-bit field */
#define C   (RT_ENDIAN*(2-A)*4)     /* for cmdx*_** working on 64-bit field */
#define D   (RT_ENDIAN*(P-1)*4)     /* for cmdw*_** working on P-size field */
#define E   (RT_ENDIAN*(P-A)*4)     /* for cmdx*_** working on P-size field */
#define F   (RT_ENDIAN*(A-1)*4)     /* for cmdw*_** working on A-size field */
#define G   (RT_ENDIAN*(2-P)*4)     /* for jmpxx_xm working on 64-bit field */
#define H   (RT_ENDIAN*(L-1)*4)     /* for cmdw*_** working on L-size field */
#define I   (RT_ENDIAN*(2-L)*4)     /* for cmdy*_** working on 64-bit field */

/******************************************************************************/
/***************************   OS, COMPILER, ARCH   ***************************/
/******************************************************************************/

/*******************************   WIN32, MSVC   ******************************/

#if   defined (RT_WIN32)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

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
#define RT_SIMD_FAST_FCTRL      1*(Q/2) /* only if AVX is among build targets */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
/* RT_SIMD_COMPAT_128 distinguishes between 128-bit AVX1 & AVX2
 * when RT_128=8 SIMD backend is present among build targets */
#define RT_SIMD_COMPAT_128      RT_SIMD_COMPAT_128_MASTER
/* RT_SIMD_COMPAT_FMR when enabled changes the default behavior
 * of fm*ps_** instruction fallbacks to honour rounding mode */
#define RT_SIMD_COMPAT_FMR      RT_SIMD_COMPAT_FMR_MASTER

#if   defined (RT_512) && (RT_512 != 0)
#define Q 4
#include "rtarch_x86_512v2.h"
#elif defined (RT_256) && (RT_256 != 0)
#define Q 2
#include "rtarch_x86_256v2.h"
#elif defined (RT_128) && (RT_128 >= 8)
#define Q 1
#include "rtarch_x86_128v8.h"
#elif defined (RT_128) && (RT_128 >= 1)
#define Q 1
#include "rtarch_x86_128v4.h"
#endif /* RT_512, RT_256, RT_128 */

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

/*******************************   LINUX, GCC   *******************************/

#elif defined (RT_LINUX) || defined (RT_WIN64)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

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
#define RT_SIMD_FAST_FCTRL      1*(Q/2) /* only if AVX is among build targets */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
/* RT_SIMD_COMPAT_128 distinguishes between 128-bit AVX1 & AVX2
 * when RT_128=8 SIMD backend is present among build targets */
#define RT_SIMD_COMPAT_128      RT_SIMD_COMPAT_128_MASTER
/* RT_SIMD_COMPAT_FMR when enabled changes the default behavior
 * of fm*ps_** instruction fallbacks to honour rounding mode */
#define RT_SIMD_COMPAT_FMR      RT_SIMD_COMPAT_FMR_MASTER

#if   defined (RT_512) && (RT_512 != 0)
#define Q 4
#include "rtarch_x86_512v2.h"
#elif defined (RT_256) && (RT_256 != 0)
#define Q 2
#include "rtarch_x86_256v2.h"
#elif defined (RT_128) && (RT_128 >= 8)
#define Q 1
#include "rtarch_x86_128v8.h"
#elif defined (RT_128) && (RT_128 >= 1)
#define Q 1
#include "rtarch_x86_128v4.h"
#endif /* RT_512, RT_256, RT_128 */

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

/* ------------------------------   X32, X64   ------------------------------ */

#elif defined (RT_X32) || defined (RT_X64)

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

#if   defined (RT_X32)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(leaq, %%rax, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
    ADR REX(0,       RXB(MD)) EMITB(0x89)                                   \
        MRM(0x00,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#elif defined (RT_X64)

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
#define RT_SIMD_FAST_FCTRL      1*(Q/2) /* only if AVX is among build targets */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
/* RT_SIMD_COMPAT_128 distinguishes between 128-bit AVX1 & AVX2
 * when RT_128=8 SIMD backend is present among build targets */
#define RT_SIMD_COMPAT_128      RT_SIMD_COMPAT_128_MASTER
/* RT_SIMD_COMPAT_FMR when enabled changes the default behavior
 * of fm*ps_** instruction fallbacks to honour rounding mode */
#define RT_SIMD_COMPAT_FMR      RT_SIMD_COMPAT_FMR_MASTER

#if   defined (RT_512) && (RT_512 != 0)
#define Q 4
#include "rtarch_x64_512v2.h"
#elif defined (RT_256) && (RT_256 != 0)
#define Q 2
#include "rtarch_x64_256v2.h"
#elif defined (RT_128) && (RT_128 >= 8)
#define Q 1
#include "rtarch_x64_128v8.h"
#elif defined (RT_128) && (RT_128 >= 1)
#define Q 1
#include "rtarch_x64_128v4.h"
#endif /* RT_512, RT_256, RT_128 */

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

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)

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
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xE5800000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1 /* takes all available regs except (SP, PC) */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
/* RT_SIMD_COMPAT_FMA when enabled changes the default behavior
 * of fmaps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMA      RT_SIMD_COMPAT_FMA_MASTER
/* RT_SIMD_COMPAT_FMS when enabled changes the default behavior
 * of fmsps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_FMS      RT_SIMD_COMPAT_FMS_MASTER
/* RT_SIMD_COMPAT_DIV when enabled changes the default behavior
 * of divps_** to the corresponding IEEE-compatible fallback */
#define RT_SIMD_COMPAT_DIV      RT_SIMD_COMPAT_DIV_MASTER
/* RT_SIMD_COMPAT_SQR when enabled changes the default behavior
 * of sqrps_** to the corresponding IEEE-compatible fallback */
#define RT_SIMD_COMPAT_SQR      RT_SIMD_COMPAT_SQR_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define Q 2
#error "AArch32 doesn't support SIMD wider than 128-bit, check build flags"
#elif defined (RT_128) && (RT_128 != 0)
#define Q 1
#include "rtarch_arm_128v4.h"
#endif /* RT_256, RT_128 */

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

#elif defined (RT_A32) || defined (RT_A64)

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

#if   defined (RT_A32)

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xB9000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#elif defined (RT_A64)

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xF9000000 | MDM(Teax,    MOD(MD), VXL(DD), B1(DD), P1(DD)))

#endif /* defined (RT_A32, RT_A64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define Q 2
#include "rtarch_a64_256v1.h"
#elif defined (RT_128) && (RT_128 != 0)
#define Q 1
#include "rtarch_a64_128v1.h"
#endif /* RT_256, RT_128 */

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
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* w22 <- (0 << 22) */

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
        EMITW(0x52A01800 | MRM(TExx, 0x00, 0x00)) /* w25 <- (3 << 22) */    \
        EMITW(0x52A01000 | MRM(TCxx, 0x00, 0x00)) /* w24 <- (2 << 22) */    \
        EMITW(0x52A00800 | MRM(TAxx, 0x00, 0x00)) /* w23 <- (1 << 22) */    \
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* w22 <- (0 << 22) */

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
        EMITW(0x52A02000 | MRM(TNxx, 0x00, 0x00)) /* w22 <- (4 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- w22 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* w22 <- (0 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- w22 */         \
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
        EMITW(0x52A03800 | MRM(TExx, 0x00, 0x00)) /* w25 <- (7 << 22) */    \
        EMITW(0x52A03000 | MRM(TCxx, 0x00, 0x00)) /* w24 <- (6 << 22) */    \
        EMITW(0x52A02800 | MRM(TAxx, 0x00, 0x00)) /* w23 <- (5 << 22) */    \
        EMITW(0x52A02000 | MRM(TNxx, 0x00, 0x00)) /* w22 <- (4 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- w22 */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0x52A00000 | MRM(TNxx, 0x00, 0x00)) /* w22 <- (0 << 22) */    \
        EMITW(0xD51B4400 | MRM(TNxx, 0x00, 0x00)) /* fpcr <- w22 */         \
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
#endif /* RT_SIMD_CODE */

/* ------------------------------   M32, M64   ------------------------------ */

#elif defined (RT_M32) || defined (RT_M64)

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

#if   defined (RT_M32)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(la, $a0, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xAC000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#elif defined (RT_M64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(dla, $a0, lb) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xFC000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#endif /* defined (RT_M32, RT_M64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define Q 2
#include "rtarch_m64_256v1.h"
#elif defined (RT_128) && (RT_128 != 0)
#define Q 1
#include "rtarch_m64_128v1.h"
#endif /* RT_256, RT_128 */

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
        movox_ld(Xmm8, Mebp, inf_GPC06_32)        /* w8  <- -0.0 32-bit */  \
        movqx_ld(Xmm9, Mebp, inf_GPC06_64)        /* w9  <- -0.0 64-bit */  \
        EMITS(0x78BE0019 | MXM(TmmS, 0x08, 0x00)) /* w28 <- w8 */           \
        EMITS(0x78BE0019 | MXM(TmmT, 0x09, 0x00)) /* w29 <- w9 */           \
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
        movox_ld(Xmm8, Mebp, inf_GPC06_32)        /* w8  <- -0.0 32-bit */  \
        movqx_ld(Xmm9, Mebp, inf_GPC06_64)        /* w9  <- -0.0 64-bit */  \
        EMITS(0x78BE0019 | MXM(TmmS, 0x08, 0x00)) /* w28 <- w8 */           \
        EMITS(0x78BE0019 | MXM(TmmT, 0x09, 0x00)) /* w29 <- w9 */           \
        EMITS(0x7860001E | MXM(TmmZ, TmmZ, TmmZ)) /* w30 <- 0 (xor) */      \
        EMITW(0x3C000000 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(0 << 24) */  \
        EMITW(0x34000001 | MRM(0x00, TNxx, TAxx)) /* r21 <- 1|(0 << 24) */  \
        EMITW(0x34000002 | MRM(0x00, TNxx, TCxx)) /* r22 <- 2|(0 << 24) */  \
        EMITW(0x34000003 | MRM(0x00, TNxx, TExx)) /* r23 <- 3|(0 << 24) */

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
        movox_ld(Xmm8, Mebp, inf_GPC06_32)        /* w8  <- -0.0 32-bit */  \
        movqx_ld(Xmm9, Mebp, inf_GPC06_64)        /* w9  <- -0.0 64-bit */  \
        EMITS(0x78BE0019 | MXM(TmmS, 0x08, 0x00)) /* w28 <- w8 */           \
        EMITS(0x78BE0019 | MXM(TmmT, 0x09, 0x00)) /* w29 <- w9 */           \
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
        movox_ld(Xmm8, Mebp, inf_GPC06_32)        /* w8  <- -0.0 32-bit */  \
        movqx_ld(Xmm9, Mebp, inf_GPC06_64)        /* w9  <- -0.0 64-bit */  \
        EMITS(0x78BE0019 | MXM(TmmS, 0x08, 0x00)) /* w28 <- w8 */           \
        EMITS(0x78BE0019 | MXM(TmmT, 0x09, 0x00)) /* w29 <- w9 */           \
        EMITS(0x7860001E | MXM(TmmZ, TmmZ, TmmZ)) /* w30 <- 0 (xor) */      \
        EMITW(0x3C000100 | MRM(0x00, 0x00, TNxx)) /* r20 <- 0|(1 << 24) */  \
        EMITW(0x34000001 | MRM(0x00, TNxx, TAxx)) /* r21 <- 1|(1 << 24) */  \
        EMITW(0x34000002 | MRM(0x00, TNxx, TCxx)) /* r22 <- 2|(1 << 24) */  \
        EMITW(0x34000003 | MRM(0x00, TNxx, TExx)) /* r23 <- 3|(1 << 24) */  \
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
#define movox_ld(XD, MS, DS)
#define movqx_ld(XD, MS, DS)
#define EMITS(w) /* EMPTY */
#else  /* RT_SIMD_CODE */
#define EMITS(w)    EMITW(w)
#endif /* RT_SIMD_CODE */

/* ------------------------------   P32, P64   ------------------------------ */

#elif defined (RT_P32) || defined (RT_P64)

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

#if   defined (RT_P32)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(lis, %%r4, lb@h) ASM_END                            \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@l) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x90000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#elif defined (RT_P64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(lis, %%r4, lb@highest) ASM_END                      \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@higher) ASM_END                 \
        ASM_BEG ASM_OP3(sldi, %%r4, %%r4, 32) ASM_END                       \
        ASM_BEG ASM_OP3(oris, %%r4, %%r4, lb@h) ASM_END                     \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@l) ASM_END

#define label_st(lb, MD, DD)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xF8000000 | MDM(Teax,    MOD(MD), VAL(DD), B1(DD), P1(DD)))

#endif /* defined (RT_P32, RT_P64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1 /* not applicable to Power */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_XMM distinguishes between SIMD reg-file sizes
 * with current top values: 0 - 16, 1 - 15, 2 - 14 SIMD regs */
#define RT_SIMD_COMPAT_XMM      RT_SIMD_COMPAT_XMM_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER
/* RT_SIMD_COMPAT_DIV when enabled changes the default behavior
 * of divps_** to the corresponding IEEE-compatible fallback */
#define RT_SIMD_COMPAT_DIV      RT_SIMD_COMPAT_DIV_MASTER
/* RT_SIMD_COMPAT_SQR when enabled changes the default behavior
 * of sqrps_** to the corresponding IEEE-compatible fallback */
#define RT_SIMD_COMPAT_SQR      RT_SIMD_COMPAT_SQR_MASTER
/* RT_BASE_COMPAT_ZFL when enabled makes setting-flags BASE ops
 * compatible with 64-bit processors running 32-bit ISA mode */
#define RT_BASE_COMPAT_ZFL      1 /* only necessary on Power */

#if   defined (RT_512) && (RT_512 != 0)
#define Q 4
#include "rtarch_p64_512v2.h"
#elif defined (RT_256) && (RT_256 != 0)
#define Q 2
#include "rtarch_p64_256v2.h"
#elif defined (RT_128) && (RT_128 >= 2)
#define Q 1
#include "rtarch_p64_128v4.h"
#elif defined (RT_128) && (RT_128 == 1)
#define Q 1
#include "rtarch_p32_128v1.h"
#endif /* RT_512, RT_256, RT_128 */

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
        sregs_sa()                                                          \
        EMITW(0x7C000278 | MSM(TZxx, TZxx, TZxx)) /* r0  <- 0 (xor) */      \
        EMITW(0x7C000040 | MRM(0x08, TLxx, TLxx)) /* cmplw cr2, r24, r24 */ \
        EMITW(0x7C0002A6 | MRM(TCxx, 0x00, 0x09)) /* ctr -> r27 */          \
        EMITS(0x7C0002A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave -> r28 */       \
        EMITS(0x3800FFFF | MRM(TIxx, 0x00, 0x00)) /* r25 <- -1 */           \
        EMITS(0x7C0003A6 | MRM(TIxx, 0x08, 0x00)) /* vrsave <- r25 */       \
        EMITS(0x1000038C | MXM(TmmQ, 0x1F, 0x00)) /* v15 <- all-ones */     \
        movox_ld(Xmm2, Mebp, inf_GPC01_32)        /* v2  <- +1.0f 32-bit */ \
        movox_ld(Xmm4, Mebp, inf_GPC02_32)        /* v4  <- -0.5f 32-bit */ \
        movox_ld(Xmm8, Mebp, inf_GPC04_32)        /* v8  <- 0x7FFFFFFF */   \
        EMITM(0x100004C4 | MXM(TmmR, TmmR, TmmR)) /* v23 <- v23 xor v23 */  \
        EMITM(0x10000504 | MXM(TmmS, 0x08, 0x08)) /* v24 <- not v8 */       \
        EMITM(0x10000484 | MXM(TmmU, 0x02, 0x02)) /* v26 <- v2 */           \
        EMITM(0x10000484 | MXM(TmmV, 0x04, 0x04)) /* v27 <- v4 */           \
        EMITP(0xF0000496 | MXM(TmmQ, 0x02, 0x02)) /* vs15 <- v2 */          \
        EMITP(0xF0000496 | MXM(TmmM, 0x04, 0x04)) /* vs31 <- v4 */

#define ASM_LEAVE(__Info__)                                                 \
        EMITW(0x7C0003A6 | MRM(TCxx, 0x00, 0x09)) /* ctr <- r27 */          \
        EMITS(0x7C0003A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave <- r28 */       \
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
        sregs_sa()                                                          \
        EMITW(0x7C000278 | MSM(TZxx, TZxx, TZxx)) /* r0  <- 0 (xor) */      \
        EMITW(0x7C000040 | MRM(0x08, TLxx, TLxx)) /* cmplw cr2, r24, r24 */ \
        EMITW(0x7C0002A6 | MRM(TCxx, 0x00, 0x09)) /* ctr -> r27 */          \
        EMITS(0x7C0002A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave -> r28 */       \
        EMITS(0x3800FFFF | MRM(TIxx, 0x00, 0x00)) /* r25 <- -1 */           \
        EMITS(0x7C0003A6 | MRM(TIxx, 0x08, 0x00)) /* vrsave <- r25 */       \
        EMITS(0x1000038C | MXM(TmmQ, 0x1F, 0x00)) /* v15 <- all-ones */     \
        movox_ld(Xmm2, Mebp, inf_GPC01_32)        /* v2  <- +1.0f 32-bit */ \
        movox_ld(Xmm4, Mebp, inf_GPC02_32)        /* v4  <- -0.5f 32-bit */ \
        movox_ld(Xmm8, Mebp, inf_GPC04_32)        /* v8  <- 0x7FFFFFFF */   \
        EMITM(0x100004C4 | MXM(TmmR, TmmR, TmmR)) /* v23 <- v23 xor v23 */  \
        EMITM(0x10000504 | MXM(TmmS, 0x08, 0x08)) /* v24 <- not v8 */       \
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
        EMITW(0x7C0003A6 | MRM(TCxx, 0x00, 0x09)) /* ctr <- r27 */          \
        EMITS(0x7C0003A6 | MRM(TVxx, 0x08, 0x00)) /* vrsave <- r28 */       \
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
#define movox_ld(XD, MS, DS)
#define movqx_ld(XD, MS, DS)
#define EMITS(w) /* EMPTY */
#define EMITM(w) /* EMPTY */
#define EMITP(w) /* EMPTY */
#elif (RT_128 >= 2) || (RT_256 != 0) || (RT_512 != 0)
#define EMITS(w)    EMITW(w)
#define EMITM(w) /* EMPTY */
#define EMITP(w)    EMITW(w)
#else  /* RT_128 < 2 */
#define EMITS(w)    EMITW(w)
#define EMITM(w)    EMITW(w)
#define EMITP(w) /* EMPTY */
#endif /* RT_SIMD_CODE */

#endif /* RT_X86, RT_X32/X64, RT_ARM, RT_A32/A64, RT_M32/M64, RT_P32/P64 */

#endif /* OS, COMPILER, ARCH */

#endif /* RT_RTARCH_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

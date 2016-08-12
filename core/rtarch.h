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
 * are exposed via (ILP32 ABI of) AArch64:ARMv8 ISA in (A32 and) A64 target(s).
 *
 * Preliminary naming scheme for legacy, current and potential future targets.
 *
 * Legacy 32-bit BASE
 *    and 32-bit SIMD targets:
 *
 *  - rtarch_arm.h         - AArch32:ARMv7 ISA, 16 BASE regs, 8 + temps used
 *  - rtarch_arm_128.h     - 32-bit elements, 16 SIMD regs, 8 + temps used
 *  - rtarch_x86.h         - x86 32-bit ISA, 8 BASE regs, 6 + esp, ebp used
 *  - rtarch_x86_128.h     - 32-bit elements, 8 SIMD regs, 8 used, SSE 128-bit
 *  - rtarch_x86_256.h     - 32-bit elements, 8 SIMD regs, 8 used, AVX 256-bit
 *
 * Current 32/64-bit BASE
 *    and 32-bit SIMD targets:
 *
 *  - rtarch_a32.h         - AArch64:ILP32 ABI, 32 BASE regs, int-div, fp-cvt-r
 *  - rtarch_a64.h         - AArch64:ARMv8 ISA, 32 BASE regs, int-div, fp-cvt-r
 *  - rtarch_a32_128.h     - 32-bit elements, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_m32.h         - MIPS32 r5/r6 ISA, 32 BASE regs, 14 + 4 used
 *  - rtarch_m64.h         - MIPS64 r5/r6 ISA, 32 BASE regs, 14 + 4 used
 *  - rtarch_m32_128.h     - 32-bit elements, 32 SIMD regs, MSA 128-bit
 *  - rtarch_p32.h         - Power 32-bit ISA, 32 BASE regs, 14 + 5 used
 *  - rtarch_p64.h         - Power 64-bit ISA, 32 BASE regs, 14 + 5 used
 *  - rtarch_p32_128.h     - 32-bit elements, 32 SIMD regs, VMX/VSX 128-bit
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 BASE regs, 32-bit ptrs
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 BASE regs, 64-bit ptrs
 *  - rtarch_x32_128.h     - 32-bit elements, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x32_256.h     - 32-bit elements, 16 SIMD regs, AVX 256-bit
 *
 * Future 32-bit SIMD targets:
 *
 *  - rtarch_x86_512.h     - 32-bit elements,  8 SIMD regs, AVX 512-bit
 *  - rtarch_x32_512.h     - 32-bit elements, 32 SIMD regs, AVX 512-bit
 *
 * Future 64-bit SIMD targets:
 *
 *  - rtarch_a64_128.h     - 64-bit elements, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_m64_128.h     - 64-bit elements, 32 SIMD regs, MSA 128-bit
 *  - rtarch_p64_128.h     - 64-bit elements, 32 SIMD regs, VMX/VSX 128-bit
 *  - rtarch_x64_128.h     - 64-bit elements, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x64_256.h     - 64-bit elements, 16 SIMD regs, AVX 256-bit
 *  - rtarch_x64_512.h     - 64-bit elements, 32 SIMD regs, AVX 512-bit
 *
 * Preliminary naming scheme for extended BASE and SIMD register files.
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
 * Although register names are fixed, register sizes are not and depend on the
 * chosen target (32/64-bit BASE and 128/256-bit SIMD are mostly implemented).
 * Base registers can be 32-bit/64-bit wide, while their SIMD counterparts
 * depend on the architecture and SIMD version chosen for the target.
 * Fractional sub-registers don't have names and aren't architecturally
 * visible in the assembler in order to simplify SPMD programming model.
 */

/******************************************************************************/
/***************************   OS, COMPILER, ARCH   ***************************/
/******************************************************************************/

/*
 * The following instruction namespaces are reserved for current/future use.
 *
 * cmdw*_** - word-size args, BASE ISA (always fixed at 32-bit)
 * cmdx*_** - addr-size args, BASE ISA (32/64-bit configurable with RT_ADDRESS)
 * cmdy*_** - SIMD-elem args, BASE ISA (32/64-bit configurable with RT_ELEMENT)
 *
 * cmdz*_** - usage for setting-flags is deprecated, reserved for fixed 64-bit
 * cmd*z_** - usage for setting-flags is implemented orthogonal to size
 *
 * cmdb*_** - byte-size args, BASE ISA (displacement/alignment may differ)
 * cmdh*_** - half-size args, BASE ISA (displacement/alignment may differ)
 *
 * cmdo*_** - SIMD-elem args, SIMD ISA (always fixed at 32-bit, packed)
 * cmdp*_** - SIMD-elem args, SIMD ISA (32/64-bit configurable, packed)
 * cmdq*_** - SIMD-elem args, SIMD ISA (always fixed at 64-bit, packed)
 *
 * cmdr*_** - SIMD-elem args, SIMD ISA (always fixed at 32-bit, scalar)
 * cmds*_** - SIMD-elem args, SIMD ISA (32/64-bit configurable, scalar)
 * cmdt*_** - SIMD-elem args, SIMD ISA (always fixed at 64-bit, scalar)
 *
 * Mixing of 64/32-bit fields in backend structures may lead to misalignment
 * of 64-bit fields to 4-byte boundary, which is not supported on some targets.
 * Place fields carefully to ensure proper alignment for all data types.
 * Note that within cmdx*_** subset most of the instructions follow in-heap
 * address size (RT_ADDRESS or A) and only label_ld/st, jmpxx_xr/xm follow
 * pointer size (RT_POINTER or P) as code/data/stack segments are fixed.
 * In 64/32-bit (ptr/adr) hybrid mode there is no way to move 64-bit registers,
 * thus label_ld has very limited use as jmpxx_xr(Reax) is the only matching op.
 * Stack ops always work with full registers regardless of the mode chosen.
 *
 * The cmdw*_** and cmdx*_** subsets are not easily compatible on all targets,
 * thus any register modified by cmdw*_** cannot be used in cmdx*_** subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subset across all targets.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subset,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (cmdw*_**, cmdx*_** or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 * Mixing of cmdw*_** and cmdx*_** without C/C++ is supported via F definition,
 * but requires two offsets for each field, with F for (w*) and plain for (x*).
 *
 * Setting-flags instructions' naming scheme may change again in the future for
 * better orthogonality with operands size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on certain targets use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
 *
 * For fixed 64-bit packed/scalar SIMD ISA there will be no BASE ISA equivalent
 * on native 32-bit processors. Applications requiring 64-bit data flow in SIMD
 * will have to be coded with BASE ISA size limitations in mind.
 * Working with sub-word BASE elements (byte, half) is reserved for future use,
 * however current displacement types may not work due to natural alignment.
 * Signed/unsigned types can be supported orthogonally in cmd*n_**, cmd*x_**.
 * Working with sub-word SIMD elements (byte, half) has not been investigated.
 * However, as current major ISAs lack the ability to do sub-word fp-compute,
 * these corresponding subsets cannot be considered valid targets for SPMD.
 */

/*
 * Master config flags for SIMD compatibility modes across all targets.
 * Each target can be configured individually regardless of the master flag.
 * Refer to individual target sections for flags' detailed description.
 */
#define RT_SIMD_FLUSH_ZERO_MASTER       0 /* optional on MIPS and Power */
#define RT_SIMD_COMPAT_RCP_MASTER       0 /* for rcpps_** full-precision */
#define RT_SIMD_COMPAT_RSQ_MASTER       0 /* for rsqps_** full-precision */
#define RT_SIMD_COMPAT_DIV_MASTER       1 /* for divps_** IEEE-compatible */
#define RT_SIMD_COMPAT_SQR_MASTER       1 /* for sqrps_** IEEE-compatible */

/*
 * Short names Q, S, W for RT_SIMD_QUADS, RT_SIMD_WIDTH, pass-through wrapper.
 * Used independently for SIMD-fields' sizes and offsets in backend structures.
 * Must be undef'd explicitly after use to avoid collisions with system headers.
 */
#undef Q /* in case Q is defined outside */
#undef S /* in case S is defined outside */
#undef W /* in case W is defined outside */

/*
 * Short names P, A, B, C, D, E, F, G for RT_POINTER/32, RT_ADDRESS/32, offsets.
 * Used independently for SIMD-fields' sizes and offsets in backend structures.
 * Must be undef'd explicitly after use to avoid collisions with system headers.
 */
#undef P /* in case P is defined outside */
#undef A /* in case A is defined outside */

#undef B /* in case B is defined outside */
#undef C /* in case C is defined outside */
#undef D /* in case D is defined outside */
#undef E /* in case E is defined outside */
#undef F /* in case F is defined outside */
#undef G /* in case G is defined outside */


#define P   (RT_POINTER/32)         /* short name for RT_POINTER/32 */
#define A   (RT_ADDRESS/32)         /* short name for RT_ADDRESS/32 */

#define B   (RT_ENDIAN*(2-1)*4)     /* for cmdw*_** working on 64-bit field */
#define C   (RT_ENDIAN*(2-A)*4)     /* for cmdx*_** working on 64-bit field */
#define D   (RT_ENDIAN*(P-1)*4)     /* for cmdw*_** working on P-size field */
#define E   (RT_ENDIAN*(P-A)*4)     /* for cmdx*_** working on P-size field */
#define F   (RT_ENDIAN*(A-1)*4)     /* for cmdw*_**, cmdx*_** without C/C++ */
#define G   (RT_ENDIAN*(2-P)*4)     /* for jmpxx_xm working on 64-bit field */

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

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        EMITB(0x89)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1*(S/8) /* only if AVX is among build targets */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#include "rtarch_x86_256.h"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_x86_128.h"
#endif /* RT_256, RT_128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a significant portion of registers onto
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, so that the ASM overhead is minimized.
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
#define mxcsr_ld(RM, DP)
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

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        EMITB(0x89)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1*(S/8) /* only if AVX is among build targets */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#include "rtarch_x86_256.h"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_x86_128.h"
#endif /* RT_256, RT_128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a significant portion of registers onto
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, so that the ASM overhead is minimized.
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
#define mxcsr_ld(RM, DP)
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

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
    ADR REX(0,       RXB(RM)) EMITB(0x89)                                   \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#elif defined (RT_X64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(leaq, %%rax, lb) ASM_END

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x89)                                   \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* defined (RT_X32, RT_X64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1*(S/8) /* only if AVX is among build targets */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#include "rtarch_x32_256.h"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_x32_128.h"
#endif /* RT_256, RT_128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a significant portion of registers onto
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, so that the ASM overhead is minimized.
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
#define mxcsr_ld(RM, DP)
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
        ASM_BEG ASM_OP2(adr, r0, lb) ASM_END

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE5800000 | MDM(Teax,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

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
/* RT_SIMD_COMPAT_DIV when enabled changes the default behavior
 * of divps_** to the corresponding IEEE-compatible fallback */
#define RT_SIMD_COMPAT_DIV      RT_SIMD_COMPAT_DIV_MASTER
/* RT_SIMD_COMPAT_SQR when enabled changes the default behavior
 * of sqrps_** to the corresponding IEEE-compatible fallback */
#define RT_SIMD_COMPAT_SQR      RT_SIMD_COMPAT_SQR_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "AArch32 doesn't support SIMD wider than 4, check build flags"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_arm_128.h"
#endif /* RT_256, RT_128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a significant portion of registers onto
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, so that the ASM overhead is minimized.
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

#if   defined (RT_A32)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(adr, x0, lb) ASM_END

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xB9000000 | MDM(Teax,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#elif defined (RT_A64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(adr, x0, lb) ASM_END

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9000000 | MDM(Teax,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#endif /* defined (RT_A32, RT_A64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "AArch64 doesn't support SIMD wider than 4, check build flags"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_a32_128.h"
#endif /* RT_256, RT_128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a significant portion of registers onto
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, so that the ASM overhead is minimized.
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

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xAC000000 | MDM(Teax,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#elif defined (RT_M64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(dla, $a0, lb) ASM_END

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xFC000000 | MDM(Teax,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#endif /* defined (RT_M32, RT_M64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
/* RT_SIMD_COMPAT_RCP when enabled changes the default behavior
 * of rcpps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RCP      RT_SIMD_COMPAT_RCP_MASTER
/* RT_SIMD_COMPAT_RSQ when enabled changes the default behavior
 * of rsqps_** instructions to their full-precision fallback */
#define RT_SIMD_COMPAT_RSQ      RT_SIMD_COMPAT_RSQ_MASTER

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "mipsMSA doesn't support SIMD wider than 4, check build flags"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_m32_128.h"
#endif /* RT_256, RT_128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a significant portion of registers onto
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, so that the ASM overhead is minimized.
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

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x90000000 | MDM(Teax,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#elif defined (RT_P64)

#define label_ld(lb)/*Reax*/                                                \
        ASM_BEG ASM_OP2(lis, %%r4, lb@highest) ASM_END                      \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@higher) ASM_END                 \
        ASM_BEG ASM_OP3(sldi, %%r4, %%r4, 32) ASM_END                       \
        ASM_BEG ASM_OP3(oris, %%r4, %%r4, lb@h) ASM_END                     \
        ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@l) ASM_END

#define label_st(lb, RM, DP)                                                \
        label_ld(lb)/*Reax*/                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF8000000 | MDM(Teax,    MOD(RM), VAL(DP), B1(DP), P1(DP)))

#endif /* defined (RT_P32, RT_P64) */

/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      1 /* not applicable to Power */
/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      RT_SIMD_FLUSH_ZERO_MASTER
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

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "AltiVec doesn't support SIMD wider than 4, check build flags"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_p32_128.h"
#endif /* RT_256, RT_128 */

/*
 * As ASM_ENTER/ASM_LEAVE save/load a significant portion of registers onto
 * the stack, they are considered heavy and therefore best suited for compute
 * intensive parts of the program, so that the ASM overhead is minimized.
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
        movpx_ld(Xmm2, Mebp, inf_GPC01)           /* v2  <- +1.0f */        \
        movpx_ld(Xmm4, Mebp, inf_GPC02)           /* v4  <- -0.5f */        \
        movpx_ld(Xmm8, Mebp, inf_GPC04)           /* v8  <- 0x7FFFFFFF */   \
        EMITS(0x10000504 | MXM(TmmS, 0x08, 0x08)) /* v24 <- not v8 */       \
        EMITS(0x10000484 | MXM(TmmQ, 0x08, TmmS)) /* v25 <- v8 or v24 */    \
        EMITS(0x10000484 | MXM(TmmA, 0x02, 0x02)) /* v26 <- v2 */           \
        EMITS(0x10000484 | MXM(TmmB, 0x04, 0x04)) /* v27 <- v4 */           \
        EMITS(0x1000004A | MXM(TmmR, TmmS, TmmS)) /* v23 <- v24 - v24 */

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
        movpx_ld(Xmm2, Mebp, inf_GPC01)           /* v2  <- +1.0f */        \
        movpx_ld(Xmm4, Mebp, inf_GPC02)           /* v4  <- -0.5f */        \
        movpx_ld(Xmm8, Mebp, inf_GPC04)           /* v8  <- 0x7FFFFFFF */   \
        EMITS(0x10000504 | MXM(TmmS, 0x08, 0x08)) /* v24 <- not v8 */       \
        EMITS(0x10000484 | MXM(TmmQ, 0x08, TmmS)) /* v25 <- v8 or v24 */    \
        EMITS(0x10000484 | MXM(TmmA, 0x02, 0x02)) /* v26 <- v2 */           \
        EMITS(0x10000484 | MXM(TmmB, 0x04, 0x04)) /* v27 <- v4 */           \
        EMITS(0x1000004A | MXM(TmmR, TmmS, TmmS)) /* v23 <- v24 - v24 */    \
        EMITW(0xFC00010C | MRM(0x1C, 0x08, 0x00)) /* fpscr <- NI(4) */      \
        EMITS(0x1000034C | MXM(Tmm1, 0x01, 0x00)) /* v31 <- splt-half(1) */ \
        EMITS(0x10000644 | MXM(0x00, 0x00, Tmm1)) /* vscr <- v31, NJ(16) */

#define ASM_LEAVE_F(__Info__)                                               \
        EMITW(0xFC00010C | MRM(0x1C, 0x00, 0x00)) /* fpscr <- NI(0) */      \
        EMITS(0x1000034C | MXM(Tmm1, 0x00, 0x00)) /* v31 <- splt-half(0) */ \
        EMITS(0x10000644 | MXM(0x00, 0x00, Tmm1)) /* vscr <- v31, NJ(16) */ \
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
#define movpx_ld(RG, RM, DP)
#define EMITS(w) /* EMPTY */
#else  /* RT_SIMD_CODE */
#define EMITS(w)    EMITW(w)
#endif /* RT_SIMD_CODE */

#endif /* RT_X86, RT_X32/X64, RT_ARM, RT_A32/A64, RT_M32/M64, RT_P32/P64 */

#endif /* OS, COMPILER, ARCH */

/******************************************************************************/
/*******************************   DEFINITIONS   ******************************/
/******************************************************************************/

/*
 * SIMD quad-factor.
 */
#define RT_SIMD_QUADS       (RT_SIMD_WIDTH / 4)
#define Q (S / 4)

/*
 * Check SIMD width correctness.
 */
#if Q != RT_SIMD_QUADS || S != RT_SIMD_WIDTH || S % 4 != 0
#error "SIMD width must be divisible by 4, check definitions"
#endif /* in case S is not expressed in quads */

#endif /* RT_RTARCH_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

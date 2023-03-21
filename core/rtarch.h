/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
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
 * while maintaining strictly defined common API.
 *
 * Definitions provided in this file are intended to hide the differences of
 * inline assembly implementations in various compilers and operating systems,
 * while definitions of particular instruction sets are delegated to the
 * corresponding companion files named rtarch_***.h for BASE instructions
 * and rtarch_***_***.h for SIMD instructions.
 *
 * At present, Intel SSE2/AVX2 (32-bit x86 ISA) and ARM NEON (32-bit ARMv7 ISA)
 * are three primary targets although wider SIMD, 64-bit addressing along with
 * more available registers and other architectures can be supported by design.
 *
 * Note that AArch32 mode of ARMv8 ISA is now part of the current ARM target,
 * as it brings BASE int-div and SIMD fp-convert with explicit round parameter,
 * while IEEE-compatible SIMD fp-arithmetic with full square root and divide
 * are planned for (ILP32 ABI of) AArch64:ARMv8 ISA in (A32 and) A64 target(s).
 *
 * Preliminary naming scheme for potential future targets.
 *
 * Current 32-bit targets:
 *  - rtarch_arm.h         - AArch32:ARMv7 ISA, 16 BASE regs, 8 + temps used
 *  - rtarch_arm_128.h     - AArch32:ARMv7 ISA, 16 SIMD regs, 8 + temps used
 *  - rtarch_x86.h         - 32-bit x86 ISA, 8 BASE regs, 6 + esp, ebp used
 *  - rtarch_x86_128.h     - 32-bit x86 ISA, 8 SIMD regs, 8 used, SSE(1,2)
 *  - rtarch_x86_256.h     - 32-bit x86 ISA, 8 SIMD regs, 8 used, AVX(1,2)
 *
 * Future 32-bit targets:
 *  - rtarch_a32.h         - AArch64:ILP32 ABI, 32 BASE regs, int-div, fp-cvt-r
 *  - rtarch_a32_128.h     - AArch64:ILP32 ABI, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 BASE regs, 32-bit ptrs
 *  - rtarch_x32_128.h     - x86_64:x32 ABI, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x32_256.h     - x86_64:x32 ABI, 16 SIMD regs, AVX 256-bit
 *  - rtarch_x32_512.h     - x86_64:x32 ABI, 32 SIMD regs, AVX 512-bit
 *
 * Future 64-bit targets:
 *  - rtarch_a64.h         - AArch64:ARMv8 ISA, 32 BASE regs, int-div, fp-cvt-r
 *  - rtarch_a64_128.h     - AArch64:ARMv8 ISA, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 BASE regs, 64-bit ptrs
 *  - rtarch_x64_128.h     - x86_64:x64 ISA, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x64_256.h     - x86_64:x64 ISA, 16 SIMD regs, AVX 256-bit
 *  - rtarch_x64_512.h     - x86_64:x64 ISA, 32 SIMD regs, AVX 512-bit
 *
 * Reserved 32-bit targets:
 *  - rtarch_m32.h         - 32-bit MIPS ISA, 32 BASE regs
 *  - rtarch_m32_128.h     - 32-bit MIPS ISA, 32 SIMD regs, MSA
 *  - rtarch_p32.h         - 32-bit PowerISA, 32 BASE regs
 *  - rtarch_p32_128.h     - 32-bit PowerISA, 32 SIMD regs, VMX/VSX
 *
 * Reserved 64-bit targets:
 *  - rtarch_m64.h         - 64-bit MIPS ISA, 32 BASE regs
 *  - rtarch_m64_128.h     - 64-bit MIPS ISA, 32 SIMD regs, MSA
 *  - rtarch_p64.h         - 64-bit PowerISA, 32 BASE regs
 *  - rtarch_p64_128.h     - 64-bit PowerISA, 32 SIMD regs, VMX/VSX
 *
 * Preliminary naming scheme for extended BASE and SIMD register files.
 *
 * Current 8 BASE and 8 SIMD registers:
 *  - Reax, Rebx, Recx, Redx, Resp, Rebp, Resi, Redi
 *  - Xmm0, Xmm1, Xmm2, Xmm3, Xmm4, Xmm5, Xmm6, Xmm7
 *
 * Future 16 BASE and 16 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegF
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmF
 *
 * Future 32 BASE and 32 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegV
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmV
 *
 * While register names are fixed, register sizes are not and depend on the
 * chosen target (only 32-bit BASE and 128/256-bit SIMD are implemented now).
 * BASE registers can be 32-bit/64-bit wide, while their SIMD counterparts
 * depend on the architecture and SIMD version chosen for the target.
 * Fractional sub-registers don't have names and aren't architecturally
 * visible in the assembler in order to simplify SPMD programming model.
 */

/******************************************************************************/
/***************************   OS, COMPILER, ARCH   ***************************/
/******************************************************************************/

/*
 * Short names Q and S for RT_SIMD_QUADS and RT_SIMD_WIDTH respectively.
 * Used independently for SIMD-fields' sizes and offsets in backend structures.
 * Must be undef'd explicitly after use to avoid collisions with system headers.
 */
#ifdef Q
#undef Q
#endif /* in case Q is defined outside */

#ifdef S
#undef S
#endif /* in case S is defined outside */

/*******************************   WIN32, MSVC   ******************************/

#if   defined (RT_WIN32)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

#define ASM_OP0(op)             op
#define ASM_OP1(op, p1)         op  p1
#define ASM_OP2(op, p1, p2)     op  p1, p2

#define ASM_BEG /*internal*/    __asm {
#define ASM_END /*internal*/    }

#define EMITB(b)                ASM_BEG ASM_OP1(_emit, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(lea, eax, lb) ASM_END
#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, eax, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, lb, eax) ASM_END

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      0
/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      0*(S/8) /* only if AVX is among build targets */

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
#define ASM_ENTER(__Info__) {int __Reax__; __asm                            \
                            {                                               \
                                movlb_st(__Reax__)                          \
                                movlb_ld(__Info__)                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(__Reax__)                          \
                            }}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; __asm                            \
                            {                                               \
                                movlb_st(__Reax__)                          \
                                movlb_ld(__Info__)                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(3*4), IH(0x7F80))  \
                                movxx_mi(Mebp, inf_FCTRL(2*4), IH(0x5F80))  \
                                movxx_mi(Mebp, inf_FCTRL(1*4), IH(0x3F80))  \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(__Reax__)                          \
                            }}

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
#define ASM_ENTER_F(__Info__) {int __Reax__; __asm                          \
                            {                                               \
                                movlb_st(__Reax__)                          \
                                movlb_ld(__Info__)                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)   movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))              \
                                stack_la()                                  \
                                movlb_ld(__Reax__)                          \
                            }}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__) {int __Reax__; __asm                          \
                            {                                               \
                                movlb_st(__Reax__)                          \
                                movlb_ld(__Info__)                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(3*4), IH(0xFF80))  \
                                movxx_mi(Mebp, inf_FCTRL(2*4), IH(0xDF80))  \
                                movxx_mi(Mebp, inf_FCTRL(1*4), IH(0xBF80))  \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)   movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))              \
                                stack_la()                                  \
                                movlb_ld(__Reax__)                          \
                            }}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef mxcsr_ld
#define mxcsr_ld(RM, DP)
#endif /* mxcsr_ld */

#endif /* RT_X86 */

/*******************************   LINUX, GCC   *******************************/

#elif defined (RT_LINUX)

/* ---------------------------------   X86   -------------------------------- */

#if   defined (RT_X86)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p2", "#p1

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(leal, %%eax, lb) ASM_END
#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(movl, %%eax, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(movl, lb, %%eax) ASM_END

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      0
/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      0*(S/8) /* only if AVX is among build targets */

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
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory"                           \
                            );}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(3*4), IH(0x7F80))  \
                                movxx_mi(Mebp, inf_FCTRL(2*4), IH(0x5F80))  \
                                movxx_mi(Mebp, inf_FCTRL(1*4), IH(0x3F80))  \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory"                           \
                            );}

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
#define ASM_ENTER_F(__Info__) {int __Reax__; asm volatile                   \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)   movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))              \
                                stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory"                           \
                            );}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__) {int __Reax__; asm volatile                   \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL(3*4), IH(0xFF80))  \
                                movxx_mi(Mebp, inf_FCTRL(2*4), IH(0xDF80))  \
                                movxx_mi(Mebp, inf_FCTRL(1*4), IH(0xBF80))  \
                                movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x9F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))

#define ASM_LEAVE_F(__Info__)   movxx_mi(Mebp, inf_FCTRL(0*4), IH(0x1F80))  \
                                mxcsr_ld(Mebp, inf_FCTRL(0*4))              \
                                stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory"                           \
                            );}

#endif /* RT_SIMD_FAST_FCTRL */

#ifndef mxcsr_ld
#define mxcsr_ld(RM, DP)
#endif /* mxcsr_ld */

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(adr, r0, lb) ASM_END
#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, r0, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, lb, r0) ASM_END

/* RT_SIMD_FLUSH_ZERO when enabled changes the default behavior
 * of ASM_ENTER/ASM_LEAVE/ROUND* to corresponding _F version */
#define RT_SIMD_FLUSH_ZERO      0
/* RT_SIMD_FAST_FCTRL saves 1 instruction on FCTRL blocks entry
 * and can be enabled if ASM_ENTER(_F)/ASM_LEAVE(_F)/ROUND*(_F)
 * with (_F) and without (_F) are not intermixed in the code */
#define RT_SIMD_FAST_FCTRL      0 /* takes all available regs except (SP, PC) */

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "ARM doesn't support SIMD wider than 4 at the moment"
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
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                EMITW(0xE3A08500) /* r8  <- (0 << 22) */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory",                          \
                                  "d0",  "d1",  "d2",  "d3",                \
                                  "d4",  "d5",  "d6",  "d7",                \
                                  "d8",  "d9",  "d10", "d11",               \
                                  "d12", "d13", "d14", "d15",               \
                                  "d16", "d17", "d18", "d19",               \
                                  "d20", "d21"                              \
                            );}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                EMITW(0xE3A0E503) /* r14 <- (3 << 22) */    \
                                EMITW(0xE3A0C502) /* r12 <- (2 << 22) */    \
                                EMITW(0xE3A0A501) /* r10 <- (1 << 22) */    \
                                EMITW(0xE3A08500) /* r8  <- (0 << 22) */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory",                          \
                                  "d0",  "d1",  "d2",  "d3",                \
                                  "d4",  "d5",  "d6",  "d7",                \
                                  "d8",  "d9",  "d10", "d11",               \
                                  "d12", "d13", "d14", "d15",               \
                                  "d16", "d17", "d18", "d19",               \
                                  "d20", "d21"                              \
                            );}

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
#define ASM_ENTER_F(__Info__) {int __Reax__; asm volatile                   \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                EMITW(0xE3A08504) /* r8  <- (4 << 22) */    \
                                EMITW(0xEEE18A10) /* fpscr <- r8 */

#define ASM_LEAVE_F(__Info__)   EMITW(0xE3A08500) /* r8  <- (0 << 22) */    \
                                EMITW(0xEEE18A10) /* fpscr <- r8 */         \
                                stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory",                          \
                                  "d0",  "d1",  "d2",  "d3",                \
                                  "d4",  "d5",  "d6",  "d7",                \
                                  "d8",  "d9",  "d10", "d11",               \
                                  "d12", "d13", "d14", "d15",               \
                                  "d16", "d17", "d18", "d19",               \
                                  "d20", "d21"                              \
                            );}

#else /* RT_SIMD_FAST_FCTRL */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER_F(__Info__) {int __Reax__; asm volatile                   \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                EMITW(0xE3A0E507) /* r14 <- (7 << 22) */    \
                                EMITW(0xE3A0C506) /* r12 <- (6 << 22) */    \
                                EMITW(0xE3A0A505) /* r10 <- (5 << 22) */    \
                                EMITW(0xE3A08504) /* r8  <- (4 << 22) */    \
                                EMITW(0xEEE18A10) /* fpscr <- r8 */

#define ASM_LEAVE_F(__Info__)   EMITW(0xE3A08500) /* r8  <- (0 << 22) */    \
                                EMITW(0xEEE18A10) /* fpscr <- r8 */         \
                                stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory",                          \
                                  "d0",  "d1",  "d2",  "d3",                \
                                  "d4",  "d5",  "d6",  "d7",                \
                                  "d8",  "d9",  "d10", "d11",               \
                                  "d12", "d13", "d14", "d15",               \
                                  "d16", "d17", "d18", "d19",               \
                                  "d20", "d21"                              \
                            );}

#endif /* RT_SIMD_FAST_FCTRL */

#endif /* RT_X86, RT_ARM */

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

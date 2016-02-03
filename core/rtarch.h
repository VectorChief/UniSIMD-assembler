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
 * corresponding companion files named rtarch_***.h for core instructions
 * and rtarch_***_***.h for SIMD instructions.
 *
 * At present, Intel SSE2/AVX2 (32-bit x86 ISA) and ARM NEON (32-bit ARMv7 ISA)
 * are three primary targets, although wider SIMD, 64-bit addressing along with
 * more available registers, and other architectures can be supported by design.
 *
 * Note that AArch32 mode of ARMv8 ISA is now part of the current ARM target,
 * as it brings integer-divide and fp-convert with explicit round parameter,
 * while IEEE-compatible fp along with SIMD versions of square root and divide
 * are planned for (ILP32 ABI of) AArch64:ARMv8 ISA in (A32 and) A64 targets.
 *
 * Preliminary naming scheme for potential future targets.
 *
 * Current 32-bit targets:
 *  - rtarch_arm.h         - AArch32:ARMv7 ISA, 16 core regs, 8 + temps used
 *  - rtarch_arm_128.h     - AArch32:ARMv7 ISA, 16 SIMD regs, 8 + temps used
 *  - rtarch_x86.h         - 32-bit x86 ISA, 8 core regs, 6 + esp, ebp used
 *  - rtarch_x86_128.h     - 32-bit x86 ISA, 8 SIMD regs, 8 used, SSE
 *  - rtarch_x86_256.h     - 32-bit x86 ISA, 8 SIMD regs, 8 used, AVX
 *
 * Future 32-bit targets:
 *  - rtarch_a32.h         - AArch64:ILP32 ABI, 32 core regs, int-div, fp-cvt-r
 *  - rtarch_a32_128.h     - AArch64:ILP32 ABI, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 core regs, 32-bit ptrs
 *  - rtarch_x32_128.h     - x86_64:x32 ABI, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x32_256.h     - x86_64:x32 ABI, 16 SIMD regs, AVX 256-bit
 *  - rtarch_x32_512.h     - x86_64:x32 ABI, 32 SIMD regs, AVX 512-bit
 *
 * Future 64-bit targets:
 *  - rtarch_a64.h         - AArch64:ARMv8 ISA, 32 core regs, int-div, fp-cvt-r
 *  - rtarch_a64_128.h     - AArch64:ARMv8 ISA, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 core regs, 64-bit ptrs
 *  - rtarch_x64_128.h     - x86_64:x64 ISA, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x64_256.h     - x86_64:x64 ISA, 16 SIMD regs, AVX 256-bit
 *  - rtarch_x64_512.h     - x86_64:x64 ISA, 32 SIMD regs, AVX 512-bit
 *
 * Reserved 32-bit targets:
 *  - rtarch_m32.h         - 32-bit MIPS ISA, 32 core regs
 *  - rtarch_m32_128.h     - 32-bit MIPS ISA, 32 SIMD regs, MSA
 *  - rtarch_p32.h         - 32-bit PowerISA, 32 core regs
 *  - rtarch_p32_128.h     - 32-bit PowerISA, 32 SIMD regs, VMX
 *
 * Reserved 64-bit targets:
 *  - rtarch_m64.h         - 64-bit MIPS ISA, 32 core regs
 *  - rtarch_m64_128.h     - 64-bit MIPS ISA, 32 SIMD regs, MSA
 *  - rtarch_p64.h         - 64-bit PowerISA, 32 core regs
 *  - rtarch_p64_128.h     - 64-bit PowerISA, 32 SIMD regs, VMX
 *
 * Preliminary naming scheme for extended core and SIMD register files.
 *
 * Current 8 core and 8 SIMD registers:
 *  - Reax, Rebx, Recx, Redx, Resp, Rebp, Resi, Redi
 *  - Xmm0, Xmm1, Xmm2, Xmm3, Xmm4, Xmm5, Xmm6, Xmm7
 *
 * Future 16 core and 16 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegF
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmF
 *
 * Future 32 core and 32 SIMD registers:
 *  - Reax, ... , Redi, Reg8, Reg9, RegA, ... , RegV
 *  - Xmm0, ... , Xmm7, Xmm8, Xmm9, XmmA, ... , XmmV
 *
 * While register names are fixed, register sizes are not and depend on the
 * chosen target (only 32-bit core and 128/256-bit SIMD are implemented now).
 * Core registers can be 32-bit/64-bit wide, while their SIMD counterparts
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

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#include "rtarch_x86_256.h"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_x86_128.h"
#endif /* RT_256, RT_128 */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; __asm                            \
                            {                                               \
                                movlb_st(__Reax__)                          \
                                movlb_ld(__Info__)                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                /* placeholder for custom ops */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(__Reax__)                          \
                            }}

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)

#error "ARM native builds for Windows are not supported yet"

#endif /* RT_X86, RT_ARM */

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

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#include "rtarch_x86_256.h"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_x86_128.h"
#endif /* RT_256, RT_128 */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                /* placeholder for custom ops */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory"                           \
                            );}

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

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "ARM doesn't support SIMD wider than 4 at the moment"
#elif defined (RT_128) && (RT_128 != 0)
#define S 4
#include "rtarch_arm_128.h"
#endif /* RT_256, RT_128 */

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                /* placeholder for custom ops */

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

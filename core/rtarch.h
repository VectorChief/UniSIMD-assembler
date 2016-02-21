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
 * as it brings hw int-div and SIMD fp-convert with explicit round parameter,
 * while IEEE-compatible SIMD fp-arithmetic with full square root and divide
 * are planned for (ILP32 ABI of) AArch64:ARMv8 ISA in (A32 and) A64 target(s).
 *
 * Preliminary naming scheme for potential future targets.
 *
 * Legacy 32-bit targets:
 *  - rtarch_arm.h         - AArch32:ARMv7 ISA, 16 BASE regs, 8 + temps used
 *  - rtarch_arm_128.h     - AArch32:ARMv7 ISA, 16 SIMD regs, 8 + temps used
 *  - rtarch_x86.h         - 32-bit x86 ISA, 8 BASE regs, 6 + esp, ebp used
 *  - rtarch_x86_128.h     - 32-bit x86 ISA, 8 SIMD regs, 8 used, SSE 128-bit
 *  - rtarch_x86_256.h     - 32-bit x86 ISA, 8 SIMD regs, 8 used, AVX 256-bit
 *
 * Current 32-bit targets:
 *  - rtarch_a32.h         - AArch64:ILP32 ABI, 32 BASE regs, int-div, fp-cvt-r
 *  - rtarch_a32_128.h     - AArch64:ILP32 ABI, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_m32.h         - MIPS32 r5/r6 ISA, 32 BASE regs, 14 + 3 used
 *  - rtarch_m32_128.h     - MIPS32 r5/r6 ISA, 32 SIMD regs, MSA 128-bit
 *  - rtarch_p32.h         - 32-bit PowerISA, 32 BASE regs
 *  - rtarch_p32_128.h     - 32-bit PowerISA, 32 SIMD regs, VMX 128-bit
 *  - rtarch_x32.h         - x86_64:x32 ABI, 16 BASE regs, 32-bit ptrs
 *  - rtarch_x32_128.h     - x86_64:x32 ABI, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x32_256.h     - x86_64:x32 ABI, 16 SIMD regs, AVX 256-bit
 *
 * Future 32-bit targets:
 *  - rtarch_x86_512.h     - 32-bit x86 ISA, 8 SIMD regs, 8 used, AVX 512-bit
 *  - rtarch_x32_512.h     - x86_64:x32 ABI, 32 SIMD regs, AVX 512-bit
 *
 * Future 64-bit targets:
 *  - rtarch_a64.h         - AArch64:ARMv8 ISA, 32 BASE regs, int-div, fp-cvt-r
 *  - rtarch_a64_128.h     - AArch64:ARMv8 ISA, 32 SIMD regs, IEEE-fp, sqr, div
 *  - rtarch_m64.h         - MIPS64 r5/r6 ISA, 32 BASE regs, 14 + 3 used
 *  - rtarch_m64_128.h     - MIPS64 r5/r6 ISA, 32 SIMD regs, MSA 128-bit
 *  - rtarch_p64.h         - 64-bit PowerISA, 32 BASE regs
 *  - rtarch_p64_128.h     - 64-bit PowerISA, 32 SIMD regs, VMX 128-bit
 *  - rtarch_x64.h         - x86_64:x64 ISA, 16 BASE regs, 64-bit ptrs
 *  - rtarch_x64_128.h     - x86_64:x64 ISA, 16 SIMD regs, SSE 128-bit
 *  - rtarch_x64_256.h     - x86_64:x64 ISA, 16 SIMD regs, AVX 256-bit
 *  - rtarch_x64_512.h     - x86_64:x64 ISA, 32 SIMD regs, AVX 512-bit
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
 * Although register names are fixed, register sizes are not and depend on the
 * chosen target (only 32-bit BASE and 128/256-bit SIMD are implemented now).
 * Base registers can be 32-bit/64-bit wide, while their SIMD counterparts
 * depend on the architecture and SIMD version chosen for the target.
 * Fractional sub-registers don't have names and aren't architecturally
 * visible in the assembler in order to simplify SPMD programming model.
 */

/******************************************************************************/
/***************************   OS, COMPILER, ARCH   ***************************/
/******************************************************************************/

/*
 * Short names Q, S, W for RT_SIMD_QUADS, RT_SIMD_WIDTH, triplet wrapper.
 * Used independently for SIMD-fields' sizes and offsets in backend structures.
 * Must be undef'd explicitly after use to avoid collisions with system headers.
 */
#ifdef Q
#undef Q
#endif /* in case Q is defined outside */

#ifdef S
#undef S
#endif /* in case S is defined outside */

#ifdef W
#undef W
#endif /* in case W is defined outside */

/*
 * Short names P, A, E for RT_POINTER/32, RT_ADDRESS/32, RT_ENDIAN*(P-A)*4.
 * Used independently for SIMD-fields' sizes and offsets in backend structures.
 * Must be undef'd explicitly after use to avoid collisions with system headers.
 */
#ifdef P
#undef P
#endif /* in case P is defined outside */

#ifdef A
#undef A
#endif /* in case A is defined outside */

#ifdef E
#undef E
#endif /* in case E is defined outside */

#define P   (RT_POINTER/32)         /* short name for RT_POINTER/32 */
#define A   (RT_ADDRESS/32)         /* short name for RT_ADDRESS/32 */
#define E   (RT_ENDIAN*(P-A)*4)     /* short name for RT_ENDIAN*(P-A)*4 */

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
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(lea, eax, lb) ASM_END

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

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {rt_word __Reax__; __asm                        \
                            {                                               \
                                movlb_st(__Reax__)                          \
                                movlb_ld(__Info__)                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL, IH(0x1F80))       \
                                /* placeholder for custom op */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(__Reax__)                          \
                            }}

#endif /* RT_X86 */

/*******************************   LINUX, GCC   *******************************/

#elif defined (RT_LINUX)

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
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(leal, %%eax, lb) ASM_END

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

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {rt_word __Reax__; asm volatile                 \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL, IH(0x1F80))       \
                                /* placeholder for custom op */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" ((rt_word)__Reax__)          \
                                : [Info_]  "r" ((rt_word)__Info__)          \
                                : "cc",  "memory"                           \
                            );}

/* ---------------------------------   X32   -------------------------------- */

#elif defined (RT_X32)

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
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(leaq, %%rax, lb) ASM_END

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

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {rt_full __Reax__; asm volatile                 \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL, IH(0x1F80))       \
                                "xor %%r15, %%r15\n" /* JMP r15 <- 0 (xor) */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" ((rt_full)__Reax__)          \
                                : [Info_]  "r" ((rt_full)__Info__)          \
                                : "cc",  "memory",                          \
                                  "xmm0",  "xmm1",  "xmm2",  "xmm3",        \
                                  "xmm4",  "xmm5",  "xmm6",  "xmm7",        \
                                  "xmm8",  "xmm9",  "xmm10", "xmm11",       \
                                  "xmm12", "xmm13", "xmm14", "xmm15"        \
                            );}

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
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(adr, r0, lb) ASM_END

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

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {rt_word __Reax__; asm volatile                 \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                "eor r4, r4, r4\n" /* TZxx (r4) <- 0 (xor) */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" ((rt_word)__Reax__)          \
                                : [Info_]  "r" ((rt_word)__Info__)          \
                                : "cc",  "memory",                          \
                                  "d0",  "d1",  "d2",  "d3",                \
                                  "d4",  "d5",  "d6",  "d7",                \
                                  "d8",  "d9",  "d10", "d11",               \
                                  "d12", "d13", "d14", "d15",               \
                                  "d16", "d17", "d18", "d19",               \
                                  "d20", "d21"                              \
                            );}

/* ---------------------------------   A32   -------------------------------- */

#elif defined (RT_A32)

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
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(adr, x0, lb) ASM_END

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "ARM doesn't support SIMD wider than 4 at the moment"
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

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {rt_full __Reax__; asm volatile                 \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                /* placeholder for custom op */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" ((rt_full)__Reax__)          \
                                : [Info_]  "r" ((rt_full)__Info__)          \
                                : "cc",  "memory",                          \
                                  "q0",  "q1",  "q2",  "q3",                \
                                  "q4",  "q5",  "q6",  "q7",                \
                                  "q8",  "q9",  "q10", "q11",               \
                                  "q12", "q13", "q14", "q15", "q31"         \
                            );}

/* ---------------------------------   M32   -------------------------------- */

#elif defined (RT_M32)

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
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(la, $a0, lb) ASM_END

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "MSA doesn't support SIMD wider than 4 at the moment"
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

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {rt_word __Reax__; asm volatile                 \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                EMITW(0x787EF79E) /* TmmZ (w30) <- 0 (xor) */

#define ASM_LEAVE(__Info__)     stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory"                           \
                            );}

/* ---------------------------------   P32   -------------------------------- */

#elif defined (RT_P32)

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
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(lis, %%r4, lb@h) ASM_END    \
                                ASM_BEG ASM_OP3(ori, %%r4, %%r4, lb@l) ASM_END

#if   defined (RT_256) && (RT_256 != 0)
#define S 8
#error "VMX doesn't support SIMD wider than 4 at the moment"
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

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {rt_word __Reax__; asm volatile                 \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                EMITW(0x7C000278) /* TZxx (r0) <- 0 (xor) */\
                                movpx_ld(Xmm2, Mebp, inf_GPC01)             \
                                movpx_ld(Xmm4, Mebp, inf_GPC02)             \
                                movpx_ld(Xmm8, Mebp, inf_GPC04)             \
                                EMITX(0x13084504)                           \
                                EMITX(0x1328C484)                           \
                                EMITX(0x13421484)                           \
                                EMITX(0x13642484)                           \
                                EMITW(0x7C0902A6 | 0x1B << 21)              \
                                "cmplw cr2, %%r24, %%r24\n"

#define ASM_LEAVE(__Info__)     EMITW(0x7C0903A6 | 0x1B << 21)              \
                                stack_la()                                  \
                                movlb_ld(%[Reax_])                          \
                                : [Reax_] "+r" (__Reax__)                   \
                                : [Info_]  "r" (__Info__)                   \
                                : "cc",  "memory", "r0", "r27"              \
                            );}

#ifndef movpx_ld
#define movpx_ld(RG, RM, DP)
#define EMITX(w) /* EMPTY */
#else  /* movpx_ld */
#define EMITX(w)    EMITW(w)
#endif /* movpx_ld */

#endif /* RT_X86, RT_X32, RT_ARM, RT_A32, RT_M32, RT_P32 */

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

/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_H
#define RT_RTARCH_H

#include "rtbase.h"

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
 * At present, Intel SSE2 (32-bit x86 ISA) and ARM NEON (32-bit ARMv7 ISA)
 * are two primary targets, although wider SIMD, 64-bit addressing along with
 * more available registers, and other architectures can be supported by design.
 *
 * Preliminary naming scheme for potential future targets.
 *
 * Current 32-bit targets:
 *  - rtarch_arm.h         - 32-bit ARMv7/8 ISA, 16 core regs, 8 + temps used
 *  - rtarch_arm_128.h     - 32-bit ARMv7/8 ISA, 16 SIMD regs, 8 + temps used
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
 * chosen target (only 32-bit core and 128-bit SIMD are implemented for now).
 * Core registers can be 32-bit/64-bit wide, while their SIMD counterparts
 * depend on the architecture and SIMD version chosen for the target.
 * Fractional sub-registers don't have names and aren't architecturally
 * visible in the assembler in order to simplify SPMD programming model.
 */

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

#define ASM_BEG /*internal*/    __asm {
#define ASM_END /*internal*/    }

#define EMITB(b)                ASM_BEG ASM_OP1(_emit, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(lea, eax, lb) ASM_END
#define movlb_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, eax, lb) ASM_END
#define movlb_st(lb)/*Reax*/    ASM_BEG ASM_OP2(mov, lb, eax) ASM_END

#include "rtarch_x86_sse.h"

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; __asm                            \
                            {                                               \
                                movlb_st(__Reax__)                          \
                                movlb_ld(__Info__)                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL, IH(0x9F80))       \
                                mxcsr_ld(Mebp, inf_FCTRL)

#define ASM_LEAVE(__Info__)     movxx_mi(Mebp, inf_FCTRL, IH(0x1F80))       \
                                mxcsr_ld(Mebp, inf_FCTRL)                   \
                                stack_la()                                  \
                                movlb_ld(__Reax__)                          \
                            }}

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

#include "rtarch_x86_sse.h"

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                movxx_mi(Mebp, inf_FCTRL, IH(0x9F80))       \
                                mxcsr_ld(Mebp, inf_FCTRL)

#define ASM_LEAVE(__Info__)     movxx_mi(Mebp, inf_FCTRL, IH(0x1F80))       \
                                mxcsr_ld(Mebp, inf_FCTRL)                   \
                                stack_la()                                  \
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

#include "rtarch_arm_mpe.h"

/* use 1 local to fix optimized builds, where locals are referenced via SP,
 * while stack ops from within the asm block aren't counted into offsets */
#define ASM_ENTER(__Info__) {int __Reax__; asm volatile                     \
                            (                                               \
                                movlb_st(%[Reax_])                          \
                                movlb_ld(%[Info_])                          \
                                stack_sa()                                  \
                                movxx_rr(Rebp, Reax)                        \
                                "eor r4, r4, r4\n" /* TZxx (r4) <- 0 (xor) */

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

/*
 * Short name Q for RT_SIMD_QUADS.
 */
#ifdef Q
#undef Q
#endif /* in case Q is defined outside */
#define Q                   RT_SIMD_QUADS

/*
 * Short name S for RT_SIMD_WIDTH.
 */
#ifdef S
#undef S
#endif /* in case S is defined outside */
#define S                   RT_SIMD_WIDTH

/*
 * Check SIMD width correctness.
 */
#if Q != RT_SIMD_QUADS || S != RT_SIMD_WIDTH || S % 4 != 0
#error "SIMD width must be divisible by 4"
#endif /* in case S is not expressed in quads */

/*
 * SIMD info structure for asm enter/leave contains internal variables
 * and general purpose constants used internally by some instructions.
 * Note that DP offsets below accept only 12-bit values (0xFFF),
 * use DH and DW for 16-bit and 32-bit SIMD offsets respectively,
 * place packed scalar fields at the top of the structs to be within DP's reach.
 * SIMD width is taken into account via S and Q defined above.
 * Structure is read-write in backend.
 */
struct rt_SIMD_INFO
{
    /* internal variables */

    rt_word fctrl;
#define inf_FCTRL           DP(0x000)

    rt_word scr00;          /* scratchpad 00 */
#define inf_SCR00           DP(0x004)

    rt_word pad01[S-2];     /* reserved, do not use! */
#define inf_PAD01           DP(0x008)

    /* general purpose constants */

    rt_real gpc01[S];       /* +1.0 */
#define inf_GPC01           DP(Q*0x010)

    rt_real gpc02[S];       /* -0.5 */
#define inf_GPC02           DP(Q*0x020)

    rt_real gpc03[S];       /* +3.0 */
#define inf_GPC03           DP(Q*0x030)

    rt_word gpc04[S];       /* 0x7FFFFFFF */
#define inf_GPC04           DP(Q*0x040)

    rt_word gpc05[S];       /* 0x3F800000 */
#define inf_GPC05           DP(Q*0x050)

    rt_real pad02[S*10];    /* reserved, do not use! */
#define inf_PAD02           DP(Q*0x060)

};

/******************************************************************************/
/************************   COMMON SIMD INSTRUCTIONS   ************************/
/******************************************************************************/

/* cbr */

/*
 * Based on the original idea by Russell Borogove (kaleja[AT]estarcion[DOT]com)
 * available at http://www.musicdsp.org/showone.php?id=206
 * converted to S-way SIMD version by VectorChief.
 */
#define cbeps_rr(RG, R1, R2, RM) /* destroys R1, R2 (temp regs) */          \
        /* cube root estimate, the exponent is divided by three             \
         * in such a way that remainder bits get shoved into                \
         * the top of the normalized mantissa */                            \
        movpx_ld(W(R2), Mebp, inf_GPC04)                                    \
        movpx_rr(W(RG), W(RM))                                              \
        andpx_rr(W(RG), W(R2))   /* exponent & mantissa in biased-127 */    \
        subpx_ld(W(RG), Mebp, inf_GPC05) /* convert to 2's complement */    \
        shrpn_ri(W(RG), IB(10))  /* RG / 1024 */                            \
        movpx_rr(W(R1), W(RG))   /* RG * 341 (next 8 ops) */                \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))                                              \
        shlpx_ri(W(R1), IB(2))                                              \
        addpx_rr(W(RG), W(R1))   /* RG * (341/1024) ~= RG * (0.333) */      \
        addpx_ld(W(RG), Mebp, inf_GPC05) /* back to biased-127 */           \
        andpx_rr(W(RG), W(R2))   /* remask exponent & mantissa */           \
        annpx_rr(W(R2), W(RM))   /* original sign */                        \
        orrpx_rr(W(RG), W(R2))   /* new exponent & mantissa, old sign */

#define cbsps_rr(RG, R1, R2, RM) /* destroys R1, R2 (temp regs) */          \
        movpx_rr(W(R1), W(RG))                                              \
        mulps_rr(W(R1), W(RG))                                              \
        movpx_rr(W(R2), W(R1))                                              \
        mulps_ld(W(R1), Mebp, inf_GPC03)                                    \
        rceps_rr(W(R1), W(R1))                                              \
        mulps_rr(W(R2), W(RG))                                              \
        subps_rr(W(R2), W(RM))                                              \
        mulps_rr(W(R2), W(R1))                                              \
        subps_rr(W(RG), W(R2))

#define cbrps_rr(RG, R1, R2, RM) /* destroys R1, R2 (temp regs) */          \
        cbeps_rr(W(RG), W(R1), W(R2), W(RM))                                \
        cbsps_rr(W(RG), W(R1), W(R2), W(RM))                                \
        cbsps_rr(W(RG), W(R1), W(R2), W(RM))                                \
        cbsps_rr(W(RG), W(R1), W(R2), W(RM))

/* rcp */

#define rcpps_rr(RG, RM) /* destroys RM */                                  \
        rceps_rr(W(RG), W(RM))                                              \
        rcsps_rr(W(RG), W(RM)) /* <- not reusable without extra temp reg */

/* rsq */

#define rsqps_rr(RG, RM) /* destroys RM */                                  \
        rseps_rr(W(RG), W(RM))                                              \
        rssps_rr(W(RG), W(RM)) /* <- not reusable without extra temp reg */

#endif /* RT_RTARCH_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

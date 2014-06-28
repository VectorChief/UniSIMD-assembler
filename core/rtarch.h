/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
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
 * At present, Intel SSE (1 and 2) and ARM MPE (aka NEON) are supported
 * in 32-bit mode, with possibility to extend support to 64-bit mode
 * along with more available registers, wider SIMD and other architectures,
 * though a significant redesign of some portions of the code might be needed.
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

#include "rtarch_x86_sse.h"

#define ASM_ENTER(info)     __asm                                           \
                            {                                               \
                                stack_sa()                                  \
                                label_ld(info)                              \
                                movxx_ld(Rebp, Oeax, PLAIN)
#define ASM_LEAVE(info)         stack_la()                                  \
                            }

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)



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

#include "rtarch_x86_sse.h"

#define ASM_ENTER(info)     asm volatile                                    \
                            (                                               \
                                stack_sa()                                  \
                                movxx_ld(Rebp, Oeax, PLAIN)
#define ASM_LEAVE(info)         stack_la()                                  \
                                :                                           \
                                : "a" (&info)                               \
                                : "cc",  "memory"                           \
                            );

/* ---------------------------------   ARM   -------------------------------- */

#elif defined (RT_ARM)

#define ASM_OP0(op)             #op
#define ASM_OP1(op, p1)         #op"  "#p1
#define ASM_OP2(op, p1, p2)     #op"  "#p1", "#p2

#define ASM_BEG /*internal*/    ""
#define ASM_END /*internal*/    "\n"

#define EMITB(b)                ASM_BEG ASM_OP1(.byte, b) ASM_END
#define label_ld(lb)/*Reax*/    ASM_BEG ASM_OP2(adr, r0, lb) ASM_END

#include "rtarch_arm_mpe.h"

#define ASM_ENTER(info)     asm volatile                                    \
                            (                                               \
                                stack_sa()                                  \
                                ASM_BEG ASM_OP2(mov, r0, %[info]) ASM_END   \
                                movxx_ld(Rebp, Oeax, PLAIN)
#define ASM_LEAVE(info)         stack_la()                                  \
                                :                                           \
                                : [info] "r" (&info)                        \
                                : "cc",  "memory",                          \
                                  "d0",  "d1",  "d2",  "d3",                \
                                  "d4",  "d5",  "d6",  "d7",                \
                                  "d8",  "d9",  "d10", "d11",               \
                                  "d12", "d13", "d14", "d15",               \
                                  "d16", "d17", "d18", "d19",               \
                                  "d20", "d21"                              \
                            );

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
 * Not to be used outside backend headers.
 */
#ifdef Q
#undef Q
#endif /* in case Q is defined outside of the engine */
#define Q                   RT_SIMD_QUADS

/*
 * Short name S for RT_SIMD_WIDTH.
 * Not to be used outside backend headers.
 */
#ifdef S
#undef S
#endif /* in case S is defined outside of the engine */
#define S                   RT_SIMD_WIDTH

/*
 * Wider SIMD are supported in backend structs (S = 8, 16 were tested),
 * although there are some places in the code (tracer.cpp, engine.cpp),
 * which still rely on S = 4 for proper operation.
 */
#if Q != RT_SIMD_QUADS || S != RT_SIMD_WIDTH || S % 4 != 0
#error "SIMD width must be divisible by 4 for QuadRay engine"
#endif /* in case S is not expressed in quads */

/*
 * SIMD info structure for asm enter/leave contains internal variables
 * and general purpose constants used internally by some instructions.
 * Note that DP offsets below accept only 12-bit values (0xFFF),
 * use DH and DW for 16-bit and 32-bit SIMD offsets respectively,
 * place packed scalar fields at the top of the structs to be within DP reach.
 * SIMD width is taken into account via S and Q defined above.
 * Structure is read-write in backend.
 */
struct rt_SIMD_INFO
{
    /* internal variables */

    rt_word fctrl;
#define inf_FCTRL           DP(0x000)

    rt_word pad01[S-1];     /* reserved, do not use! */
#define inf_PAD01           DP(0x004)

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

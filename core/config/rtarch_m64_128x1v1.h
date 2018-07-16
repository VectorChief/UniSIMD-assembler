/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M64_128X1V1_H
#define RT_RTARCH_M64_128X1V1_H

#include "rtarch_m32_128x1v1.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m64_128x1v1.h: Implementation of MIPS fp64 MSA instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdp*_ri - applies [cmd] to [p]acked: [r]egister from [i]mmediate
 * cmdp*_rr - applies [cmd] to [p]acked: [r]egister from [r]egister
 *
 * cmdp*_rm - applies [cmd] to [p]acked: [r]egister from [m]emory
 * cmdp*_ld - applies [cmd] to [p]acked: as above
 *
 * cmdi*_** - applies [cmd] to 32-bit SIMD element args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit SIMD element args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size SIMD element args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit SIMD element args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit SIMD element args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size SIMD element args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD element args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size SIMD element args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit SIMD element args, packed-var-len
 *
 * cmd*x_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
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
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and fisrt source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and fisrt source
 * RS - BASE register serving as second source (first if any)
 * RT - BASE register serving as third source (second if any)
 *
 * MD - BASE addressing mode (Oeax, M***, I***) (memory-dest)
 * MG - BASE addressing mode (Oeax, M***, I***) (memory-dsrc)
 * MS - BASE addressing mode (Oeax, M***, I***) (memory-src2)
 * MT - BASE addressing mode (Oeax, M***, I***) (memory-src3)
 *
 * DD - displacement value (DP, DF, DG, DH, DV) (memory-dest)
 * DG - displacement value (DP, DF, DG, DH, DV) (memory-dsrc)
 * DS - displacement value (DP, DF, DG, DH, DV) (memory-src2)
 * DT - displacement value (DP, DF, DG, DH, DV) (memory-src3)
 *
 * IS - immediate value (is used as a second or first source)
 * IT - immediate value (is used as a third or second source)
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#if (defined RT_SIMD_CODE)

#if (RT_128X1 != 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   MSA   ***********************************/
/******************************************************************************/

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movjx_rr(XD, XS)                                                    \
        EMITW(0x78BE0019 | MXM(REG(XD), REG(XS), 0x00))

#define movjx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movjx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x78000027 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked elems */

#define mmvjx_rr(XG, XS)                                                    \
        EMITW(0x7880001E | MXM(REG(XG), REG(XS), Tmm0))

#define mmvjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7880001E | MXM(REG(XG), TmmM,    Tmm0))

#define mmvjx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7880001E | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x78000027 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andjx_rr(XG, XS)                                                    \
        andjx3rr(W(XG), W(XG), W(XS))

#define andjx_ld(XG, MS, DS)                                                \
        andjx3ld(W(XG), W(XG), W(MS), W(DS))

#define andjx3rr(XD, XS, XT)                                                \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), REG(XT)))

#define andjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annjx_rr(XG, XS)                                                    \
        EMITW(0x78C0001E | MXM(REG(XG), REG(XS), TmmZ))

#define annjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001E | MXM(REG(XG), TmmM,    TmmZ))

#define annjx3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        annjx_rr(W(XD), W(XT))

#define annjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        annjx_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrjx_rr(XG, XS)                                                    \
        orrjx3rr(W(XG), W(XG), W(XS))

#define orrjx_ld(XG, MS, DS)                                                \
        orrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrjx3rr(XD, XS, XT)                                                \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), REG(XT)))

#define orrjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornjx_rr(XG, XS)                                                    \
        notjx_rx(W(XG))                                                     \
        orrjx_rr(W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        notjx_rx(W(XG))                                                     \
        orrjx_ld(W(XG), W(MS), W(DS))

#define ornjx3rr(XD, XS, XT)                                                \
        notjx_rr(W(XD), W(XS))                                              \
        orrjx_rr(W(XD), W(XT))

#define ornjx3ld(XD, XS, MT, DT)                                            \
        notjx_rr(W(XD), W(XS))                                              \
        orrjx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorjx_rr(XG, XS)                                                    \
        xorjx3rr(W(XG), W(XG), W(XS))

#define xorjx_ld(XG, MS, DS)                                                \
        xorjx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorjx3rr(XD, XS, XT)                                                \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), REG(XT)))

#define xorjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notjx_rx(XG)                                                        \
        notjx_rr(W(XG), W(XG))

#define notjx_rr(XD, XS)                                                    \
        EMITW(0x7840001E | MXM(REG(XD), TmmZ,    REG(XS)))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negjs_rx(XG)                                                        \
        negjs_rr(W(XG), W(XG))

#define negjs_rr(XD, XS)                                                    \
        movjx_xm(Mebp, inf_GPC06_64)                                        \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), TmmM))

#define movjx_xm(MS, DS) /* not portable, do not use outside */             \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addjs_rr(XG, XS)                                                    \
        addjs3rr(W(XG), W(XG), W(XS))

#define addjs_ld(XG, MS, DS)                                                \
        addjs3ld(W(XG), W(XG), W(MS), W(DS))

#define addjs3rr(XD, XS, XT)                                                \
        EMITW(0x7820001B | MXM(REG(XD), REG(XS), REG(XT)))

#define addjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7820001B | MXM(REG(XD), REG(XS), TmmM))

#define adpjs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        adpjs_rx(W(XG))

#define adpjs_ld(XG, MS, DS)                                                \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        adpjs_rx(W(XG))

#define adpjs_rx(XG) /* not portable, do not use outside */                 \
        movts_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addts_ld(W(XG), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        addts_ld(W(XG), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XG), Mebp, inf_SCR01(0x08))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define adpjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        adpjs_rr(W(XD), W(XT))

#define adpjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        adpjs_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subjs_rr(XG, XS)                                                    \
        subjs3rr(W(XG), W(XG), W(XS))

#define subjs_ld(XG, MS, DS)                                                \
        subjs3ld(W(XG), W(XG), W(MS), W(DS))

#define subjs3rr(XD, XS, XT)                                                \
        EMITW(0x7860001B | MXM(REG(XD), REG(XS), REG(XT)))

#define subjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7860001B | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define muljs_rr(XG, XS)                                                    \
        muljs3rr(W(XG), W(XG), W(XS))

#define muljs_ld(XG, MS, DS)                                                \
        muljs3ld(W(XG), W(XG), W(MS), W(DS))

#define muljs3rr(XD, XS, XT)                                                \
        EMITW(0x78A0001B | MXM(REG(XD), REG(XS), REG(XT)))

#define muljs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x78A0001B | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divjs_rr(XG, XS)                                                    \
        divjs3rr(W(XG), W(XG), W(XS))

#define divjs_ld(XG, MS, DS)                                                \
        divjs3ld(W(XG), W(XG), W(MS), W(DS))

#define divjs3rr(XD, XS, XT)                                                \
        EMITW(0x78E0001B | MXM(REG(XD), REG(XS), REG(XT)))

#define divjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x78E0001B | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrjs_rr(XD, XS)                                                    \
        EMITW(0x7B27001E | MXM(REG(XD), REG(XS), 0x00))

#define sqrjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B27001E | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcejs_rr(XD, XS)                                                    \
        EMITW(0x7B2B001E | MXM(REG(XD), REG(XS), 0x00))

#define rcsjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsejs_rr(XD, XS)                                                    \
        EMITW(0x7B29001E | MXM(REG(XD), REG(XS), 0x00))

#define rssjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmajs_rr(XG, XS, XT)                                                \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsjs_rr(XG, XS, XT)                                                \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minjs_rr(XG, XS)                                                    \
        minjs3rr(W(XG), W(XG), W(XS))

#define minjs_ld(XG, MS, DS)                                                \
        minjs3ld(W(XG), W(XG), W(MS), W(DS))

#define minjs3rr(XD, XS, XT)                                                \
        EMITW(0x7B20001B | MXM(REG(XD), REG(XS), REG(XT)))

#define minjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7B20001B | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxjs_rr(XG, XS)                                                    \
        maxjs3rr(W(XG), W(XG), W(XS))

#define maxjs_ld(XG, MS, DS)                                                \
        maxjs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxjs3rr(XD, XS, XT)                                                \
        EMITW(0x7BA0001B | MXM(REG(XD), REG(XS), REG(XT)))

#define maxjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7BA0001B | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqjs_rr(XG, XS)                                                    \
        ceqjs3rr(W(XG), W(XG), W(XS))

#define ceqjs_ld(XG, MS, DS)                                                \
        ceqjs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqjs3rr(XD, XS, XT)                                                \
        EMITW(0x78A0001A | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x78A0001A | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnejs_rr(XG, XS)                                                    \
        cnejs3rr(W(XG), W(XG), W(XS))

#define cnejs_ld(XG, MS, DS)                                                \
        cnejs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnejs3rr(XD, XS, XT)                                                \
        EMITW(0x78E0001C | MXM(REG(XD), REG(XS), REG(XT)))

#define cnejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x78E0001C | MXM(REG(XD), REG(XS), TmmM))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltjs_rr(XG, XS)                                                    \
        cltjs3rr(W(XG), W(XG), W(XS))

#define cltjs_ld(XG, MS, DS)                                                \
        cltjs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltjs3rr(XD, XS, XT)                                                \
        EMITW(0x7920001A | MXM(REG(XD), REG(XS), REG(XT)))

#define cltjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7920001A | MXM(REG(XD), REG(XS), TmmM))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clejs_rr(XG, XS)                                                    \
        clejs3rr(W(XG), W(XG), W(XS))

#define clejs_ld(XG, MS, DS)                                                \
        clejs3ld(W(XG), W(XG), W(MS), W(DS))

#define clejs3rr(XD, XS, XT)                                                \
        EMITW(0x79A0001A | MXM(REG(XD), REG(XS), REG(XT)))

#define clejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x79A0001A | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtjs_rr(XG, XS)                                                    \
        cgtjs3rr(W(XG), W(XG), W(XS))

#define cgtjs_ld(XG, MS, DS)                                                \
        cgtjs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtjs3rr(XD, XS, XT)                                                \
        EMITW(0x7920001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgtjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7920001A | MXM(REG(XD), TmmM,    REG(XS)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgejs_rr(XG, XS)                                                    \
        cgejs3rr(W(XG), W(XG), W(XS))

#define cgejs_ld(XG, MS, DS)                                                \
        cgejs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgejs3rr(XD, XS, XT)                                                \
        EMITW(0x79A0001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x79A0001A | MXM(REG(XD), TmmM,    REG(XS)))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_128  MN64_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_128  MF64_128   /*  all satisfy the condition */

/* #define S0(mask)    S1(mask)            (defined in 32_128-bit header) */
/* #define S1(mask)    S##mask             (defined in 32_128-bit header) */

#define SMN64_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2( bz.v, xs, lb) ASM_END

#define SMF64_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(bnz.w, xs, lb) ASM_END

#define mkjjx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, MOD(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##64_128), EMPTY2)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
        cvzjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#define cvzjs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x7B23001E | MXM(REG(XD), REG(XS), 0x00))

#define cvzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B23001E | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpjs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rnpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cvpjs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmjs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rnmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cvmjs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
        rndjs_rr(W(XD), W(XS))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
        rndjs_ld(W(XD), W(MS), W(DS))

#define cvnjs_rr(XD, XS)     /* round towards near */                       \
        cvtjs_rr(W(XD), W(XS))

#define cvnjs_ld(XD, MS, DS) /* round towards near */                       \
        cvtjs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnjn_rr(XD, XS)     /* round towards near */                       \
        cvtjn_rr(W(XD), W(XS))

#define cvnjn_ld(XD, MS, DS) /* round towards near */                       \
        cvtjn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndjs_rr(XD, XS)                                                    \
        EMITW(0x7B2D001E | MXM(REG(XD), REG(XS), 0x00))

#define rndjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B2D001E | MXM(REG(XD), TmmM,    0x00))

#define cvtjs_rr(XD, XS)                                                    \
        EMITW(0x7B39001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B39001E | MXM(REG(XD), TmmM,    0x00))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtjn_rr(XD, XS)                                                    \
        EMITW(0x7B3D001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtjn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B3D001E | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrjs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvrjs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addjx_rr(XG, XS)                                                    \
        addjx3rr(W(XG), W(XG), W(XS))

#define addjx_ld(XG, MS, DS)                                                \
        addjx3ld(W(XG), W(XG), W(MS), W(DS))

#define addjx3rr(XD, XS, XT)                                                \
        EMITW(0x7860000E | MXM(REG(XD), REG(XS), REG(XT)))

#define addjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7860000E | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subjx_rr(XG, XS)                                                    \
        subjx3rr(W(XG), W(XG), W(XS))

#define subjx_ld(XG, MS, DS)                                                \
        subjx3ld(W(XG), W(XG), W(MS), W(DS))

#define subjx3rr(XD, XS, XT)                                                \
        EMITW(0x78E0000E | MXM(REG(XD), REG(XS), REG(XT)))

#define subjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x78E0000E | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shljx_ri(XG, IS)                                                    \
        shljx3ri(W(XG), W(XG), W(IS))

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        shljx3ld(W(XG), W(XG), W(MS), W(DS))

#define shljx3ri(XD, XS, IT)                                                \
        EMITW(0x78000009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x3F & VAL(IT)) << 16)

#define shljx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7B03001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7860000D | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrjx_ri(XG, IS)                                                    \
        shrjx3ri(W(XG), W(XG), W(IS))

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        shrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrjx3ri(XD, XS, IT)                                                \
        EMITW(0x79000009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x3F & VAL(IT)) << 16)

#define shrjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7B03001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7960000D | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrjn_ri(XG, IS)                                                    \
        shrjn3ri(W(XG), W(XG), W(IS))

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        shrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrjn3ri(XD, XS, IT)                                                \
        EMITW(0x78800009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x3F & VAL(IT)) << 16)

#define shrjn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7B03001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x78E0000D | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx3rr(W(XG), W(XG), W(XS))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx3ld(W(XG), W(XG), W(MS), W(DS))

#define svljx3rr(XD, XS, XT)                                                \
        EMITW(0x7860000D | MXM(REG(XD), REG(XS), REG(XT)))

#define svljx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7860000D | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx3rr(W(XG), W(XG), W(XS))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjx3rr(XD, XS, XT)                                                \
        EMITW(0x7960000D | MXM(REG(XD), REG(XS), REG(XT)))

#define svrjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7960000D | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn3rr(W(XG), W(XG), W(XS))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjn3rr(XD, XS, XT)                                                \
        EMITW(0x78E0000D | MXM(REG(XD), REG(XS), REG(XT)))

#define svrjn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x78E0000D | MXM(REG(XD), REG(XS), TmmM))

/*********   scalar double-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movts_rr(XD, XS)                                                    \
        EMITW(0x46200006 | MXM(REG(XD), REG(XS), 0x00))

#define movts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(REG(XD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movts_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xF4000000 | MDM(REG(XS), MOD(MD), VAL(DD), B1(DD), P1(DD)))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addts_rr(XG, XS)                                                    \
        addts3rr(W(XG), W(XG), W(XS))

#define addts_ld(XG, MS, DS)                                                \
        addts3ld(W(XG), W(XG), W(MS), W(DS))

#define addts3rr(XD, XS, XT)                                                \
        EMITW(0x46200000 | MXM(REG(XD), REG(XS), REG(XT)))

#define addts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200000 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subts_rr(XG, XS)                                                    \
        subts3rr(W(XG), W(XG), W(XS))

#define subts_ld(XG, MS, DS)                                                \
        subts3ld(W(XG), W(XG), W(MS), W(DS))

#define subts3rr(XD, XS, XT)                                                \
        EMITW(0x46200001 | MXM(REG(XD), REG(XS), REG(XT)))

#define subts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200001 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mults_rr(XG, XS)                                                    \
        mults3rr(W(XG), W(XG), W(XS))

#define mults_ld(XG, MS, DS)                                                \
        mults3ld(W(XG), W(XG), W(MS), W(DS))

#define mults3rr(XD, XS, XT)                                                \
        EMITW(0x46200002 | MXM(REG(XD), REG(XS), REG(XT)))

#define mults3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200002 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divts_rr(XG, XS)                                                    \
        divts3rr(W(XG), W(XG), W(XS))

#define divts_ld(XG, MS, DS)                                                \
        divts3ld(W(XG), W(XG), W(MS), W(DS))

#define divts3rr(XD, XS, XT)                                                \
        EMITW(0x46200003 | MXM(REG(XD), REG(XS), REG(XT)))

#define divts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200003 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrts_rr(XD, XS)                                                    \
        EMITW(0x46200004 | MXM(REG(XD), REG(XS), 0x00))

#define sqrts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x46200004 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcets_rr(XD, XS)                                                    \
        EMITW(0x46200015 | MXM(REG(XD), REG(XS), 0x00))

#define rcsts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsets_rr(XD, XS)                                                    \
        EMITW(0x46200016 | MXM(REG(XD), REG(XS), 0x00))

#define rssts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

#else  /* r6 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMITW(0x46200018 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200018 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMITW(0x46200019 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200019 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* r6 */

/*************   scalar double-precision floating-point compare   *************/

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define mints_rr(XG, XS)                                                    \
        mints3rr(W(XG), W(XG), W(XS))

#define mints_ld(XG, MS, DS)                                                \
        mints3ld(W(XG), W(XG), W(MS), W(DS))

#define mints3rr(XD, XS, XT)                                                \
        EMITW(0x7B20001B | MXM(REG(XD), REG(XS), REG(XT)))

#define mints3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7B20001B | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxts_rr(XG, XS)                                                    \
        maxts3rr(W(XG), W(XG), W(XS))

#define maxts_ld(XG, MS, DS)                                                \
        maxts3ld(W(XG), W(XG), W(MS), W(DS))

#define maxts3rr(XD, XS, XT)                                                \
        EMITW(0x7BA0001B | MXM(REG(XD), REG(XS), REG(XT)))

#define maxts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7BA0001B | MXM(REG(XD), REG(XS), TmmM))

#else  /* r6 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define mints_rr(XG, XS)                                                    \
        mints3rr(W(XG), W(XG), W(XS))

#define mints_ld(XG, MS, DS)                                                \
        mints3ld(W(XG), W(XG), W(MS), W(DS))

#define mints3rr(XD, XS, XT)                                                \
        EMITW(0x4620001C | MXM(REG(XD), REG(XS), REG(XT)))

#define mints3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x4620001C | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxts_rr(XG, XS)                                                    \
        maxts3rr(W(XG), W(XG), W(XS))

#define maxts_ld(XG, MS, DS)                                                \
        maxts3ld(W(XG), W(XG), W(MS), W(DS))

#define maxts3rr(XD, XS, XT)                                                \
        EMITW(0x4620001E | MXM(REG(XD), REG(XS), REG(XT)))

#define maxts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x4620001E | MXM(REG(XD), REG(XS), TmmM))

#endif /* r6 */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqts_rr(XG, XS)                                                    \
        ceqts3rr(W(XG), W(XG), W(XS))

#define ceqts_ld(XG, MS, DS)                                                \
        ceqts3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqts3rr(XD, XS, XT)                                                \
        EMITW(0x78A0001A | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x78A0001A | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnets_rr(XG, XS)                                                    \
        cnets3rr(W(XG), W(XG), W(XS))

#define cnets_ld(XG, MS, DS)                                                \
        cnets3ld(W(XG), W(XG), W(MS), W(DS))

#define cnets3rr(XD, XS, XT)                                                \
        EMITW(0x78E0001C | MXM(REG(XD), REG(XS), REG(XT)))

#define cnets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x78E0001C | MXM(REG(XD), REG(XS), TmmM))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltts_rr(XG, XS)                                                    \
        cltts3rr(W(XG), W(XG), W(XS))

#define cltts_ld(XG, MS, DS)                                                \
        cltts3ld(W(XG), W(XG), W(MS), W(DS))

#define cltts3rr(XD, XS, XT)                                                \
        EMITW(0x7920001A | MXM(REG(XD), REG(XS), REG(XT)))

#define cltts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7920001A | MXM(REG(XD), REG(XS), TmmM))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clets_rr(XG, XS)                                                    \
        clets3rr(W(XG), W(XG), W(XS))

#define clets_ld(XG, MS, DS)                                                \
        clets3ld(W(XG), W(XG), W(MS), W(DS))

#define clets3rr(XD, XS, XT)                                                \
        EMITW(0x79A0001A | MXM(REG(XD), REG(XS), REG(XT)))

#define clets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x79A0001A | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtts_rr(XG, XS)                                                    \
        cgtts3rr(W(XG), W(XG), W(XS))

#define cgtts_ld(XG, MS, DS)                                                \
        cgtts3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtts3rr(XD, XS, XT)                                                \
        EMITW(0x7920001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgtts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7920001A | MXM(REG(XD), TmmM,    REG(XS)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgets_rr(XG, XS)                                                    \
        cgets3rr(W(XG), W(XG), W(XS))

#define cgets_ld(XG, MS, DS)                                                \
        cgets3ld(W(XG), W(XG), W(MS), W(DS))

#define cgets3rr(XD, XS, XT)                                                \
        EMITW(0x79A0001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x79A0001A | MXM(REG(XD), TmmM,    REG(XS)))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M64_128X1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

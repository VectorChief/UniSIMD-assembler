/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHB_128X1V1_H
#define RT_RTARCH_AHB_128X1V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHB_128x1v1.h: Implementation of AArch64 half+byte NEON instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdg*_rx - applies [cmd] to 128-bit packed-half: [r]egister (one operand)
 * cmdg*_rr - applies [cmd] to 128-bit packed-half: [r]egister from [r]egister
 *
 * cmdg*_rm - applies [cmd] to 128-bit packed-half: [r]egister from [m]emory
 * cmdg*_ld - applies [cmd] to 128-bit packed-half: as above (friendly alias)
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and first source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and first source
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
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))

#define movgx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movgx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3C800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgx_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))

#define mmvgx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))

#define mmvgx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3C800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
        andgx3rr(W(XG), W(XG), W(XS))

#define andgx_ld(XG, MS, DS)                                                \
        andgx3ld(W(XG), W(XG), W(MS), W(DS))

#define andgx3rr(XD, XS, XT)                                                \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define andgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anngx_rr(XG, XS)                                                    \
        anngx3rr(W(XG), W(XG), W(XS))

#define anngx_ld(XG, MS, DS)                                                \
        anngx3ld(W(XG), W(XG), W(MS), W(DS))

#define anngx3rr(XD, XS, XT)                                                \
        EMITW(0x4E601C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define anngx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E601C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrgx_rr(XG, XS)                                                    \
        orrgx3rr(W(XG), W(XG), W(XS))

#define orrgx_ld(XG, MS, DS)                                                \
        orrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrgx3rr(XD, XS, XT)                                                \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orngx_rr(XG, XS)                                                    \
        orngx3rr(W(XG), W(XG), W(XS))

#define orngx_ld(XG, MS, DS)                                                \
        orngx3ld(W(XG), W(XG), W(MS), W(DS))

#define orngx3rr(XD, XS, XT)                                                \
        EMITW(0x4EE01C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define orngx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE01C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorgx_rr(XG, XS)                                                    \
        xorgx3rr(W(XG), W(XG), W(XS))

#define xorgx_ld(XG, MS, DS)                                                \
        xorgx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        notgx_rr(W(XG), W(XG))

#define notgx_rr(XD, XS)                                                    \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XS), 0x00))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
        addgx3rr(W(XG), W(XG), W(XS))

#define addgx_ld(XG, MS, DS)                                                \
        addgx3ld(W(XG), W(XG), W(MS), W(DS))

#define addgx3rr(XD, XS, XT)                                                \
        EMITW(0x4E608400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E608400 | MXM(REG(XD), REG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgx_rr(XG, XS)                                                    \
        adsgx3rr(W(XG), W(XG), W(XS))

#define adsgx_ld(XG, MS, DS)                                                \
        adsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E600C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define adsgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E600C00 | MXM(REG(XD), REG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgn_rr(XG, XS)                                                    \
        adsgn3rr(W(XG), W(XG), W(XS))

#define adsgn_ld(XG, MS, DS)                                                \
        adsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgn3rr(XD, XS, XT)                                                \
        EMITW(0x4E600C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define adsgn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E600C00 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
        subgx3rr(W(XG), W(XG), W(XS))

#define subgx_ld(XG, MS, DS)                                                \
        subgx3ld(W(XG), W(XG), W(MS), W(DS))

#define subgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E608400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E608400 | MXM(REG(XD), REG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgx_rr(XG, XS)                                                    \
        sbsgx3rr(W(XG), W(XG), W(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
        sbsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E602C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define sbsgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E602C00 | MXM(REG(XD), REG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgn_rr(XG, XS)                                                    \
        sbsgn3rr(W(XG), W(XG), W(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
        sbsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgn3rr(XD, XS, XT)                                                \
        EMITW(0x4E602C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define sbsgn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E602C00 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
        mulgx3rr(W(XG), W(XG), W(XS))

#define mulgx_ld(XG, MS, DS)                                                \
        mulgx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgx3rr(XD, XS, XT)                                                \
        EMITW(0x4E609C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E609C00 | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)     /* emits shift-right with out-of-range args */ \
        shlgx3ri(W(XG), W(XG), W(IS))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgx3ri(XD, XS, IT)                                                \
        EMITW(0x4F100400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 16) & 0x00005000) | (M(VAL(IT) > 15) & 0x20000000) |   \
        (M(VAL(IT) < 16) & ((0x0F & VAL(IT)) << 16)))

#define shlgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E020400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrgx3ri(W(XG), W(XG), W(IS))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgx3ri(XD, XS, IT)                                                \
        EMITW(0x4F100400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))

#define shrgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E020400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrgn3ri(W(XG), W(XG), W(IS))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgn3ri(XD, XS, IT)                                                \
        EMITW(0x4F100400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))

#define shrgn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E020400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E604400 | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx3rr(W(XG), W(XG), W(XS))

#define svlgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), REG(XT)))

#define svlgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx3rr(W(XG), W(XG), W(XS))

#define svrgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E60B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))

#define svrgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn3rr(W(XG), W(XG), W(XS))

#define svrgn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgn3rr(XD, XS, XT)                                                \
        EMITW(0x6E60B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x4E604400 | MXM(REG(XD), REG(XS), TmmM))

#define svrgn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E604400 | MXM(REG(XD), REG(XS), TmmM))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
        mingx3rr(W(XG), W(XG), W(XS))

#define mingx_ld(XG, MS, DS)                                                \
        mingx3ld(W(XG), W(XG), W(MS), W(DS))

#define mingx3rr(XD, XS, XT)                                                \
        EMITW(0x6E606C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mingx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E606C00 | MXM(REG(XD), REG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingn_rr(XG, XS)                                                    \
        mingn3rr(W(XG), W(XG), W(XS))

#define mingn_ld(XG, MS, DS)                                                \
        mingn3ld(W(XG), W(XG), W(MS), W(DS))

#define mingn3rr(XD, XS, XT)                                                \
        EMITW(0x4E606C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mingn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E606C00 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
        maxgx3rr(W(XG), W(XG), W(XS))

#define maxgx_ld(XG, MS, DS)                                                \
        maxgx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E606400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E606400 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgn_rr(XG, XS)                                                    \
        maxgn3rr(W(XG), W(XG), W(XS))

#define maxgn_ld(XG, MS, DS)                                                \
        maxgn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgn3rr(XD, XS, XT)                                                \
        EMITW(0x4E606400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxgn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E606400 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgx_rr(XG, XS)                                                    \
        ceqgx3rr(W(XG), W(XG), W(XS))

#define ceqgx_ld(XG, MS, DS)                                                \
        ceqgx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegx_rr(XG, XS)                                                    \
        cnegx3rr(W(XG), W(XG), W(XS))

#define cnegx_ld(XG, MS, DS)                                                \
        cnegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegx3rr(XD, XS, XT)                                                \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cnegx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgx_rr(XG, XS)                                                    \
        cltgx3rr(W(XG), W(XG), W(XS))

#define cltgx_ld(XG, MS, DS)                                                \
        cltgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E603400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E603400 | MXM(REG(XD), TmmM,    REG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgn_rr(XG, XS)                                                    \
        cltgn3rr(W(XG), W(XG), W(XS))

#define cltgn_ld(XG, MS, DS)                                                \
        cltgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgn3rr(XD, XS, XT)                                                \
        EMITW(0x4E603400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltgn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E603400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegx_rr(XG, XS)                                                    \
        clegx3rr(W(XG), W(XG), W(XS))

#define clegx_ld(XG, MS, DS)                                                \
        clegx3ld(W(XG), W(XG), W(MS), W(DS))

#define clegx3rr(XD, XS, XT)                                                \
        EMITW(0x6E603C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define clegx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E603C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegn_rr(XG, XS)                                                    \
        clegn3rr(W(XG), W(XG), W(XS))

#define clegn_ld(XG, MS, DS)                                                \
        clegn3ld(W(XG), W(XG), W(MS), W(DS))

#define clegn3rr(XD, XS, XT)                                                \
        EMITW(0x4E603C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define clegn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E603C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgx_rr(XG, XS)                                                    \
        cgtgx3rr(W(XG), W(XG), W(XS))

#define cgtgx_ld(XG, MS, DS)                                                \
        cgtgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgx3rr(XD, XS, XT)                                                \
        EMITW(0x6E603400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtgx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E603400 | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgn_rr(XG, XS)                                                    \
        cgtgn3rr(W(XG), W(XG), W(XS))

#define cgtgn_ld(XG, MS, DS)                                                \
        cgtgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgn3rr(XD, XS, XT)                                                \
        EMITW(0x4E603400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtgn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E603400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegx_rr(XG, XS)                                                    \
        cgegx3rr(W(XG), W(XG), W(XS))

#define cgegx_ld(XG, MS, DS)                                                \
        cgegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegx3rr(XD, XS, XT)                                                \
        EMITW(0x6E603C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgegx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E603C00 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegn_rr(XG, XS)                                                    \
        cgegn3rr(W(XG), W(XG), W(XS))

#define cgegn_ld(XG, MS, DS)                                                \
        cgegn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegn3rr(XD, XS, XT)                                                \
        EMITW(0x4E603C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgegn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E603C00 | MXM(REG(XD), REG(XS), TmmM))

/****************   packed byte-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgb_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))

#define movgb_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movgb_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3C800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgb_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))

#define mmvgb_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))

#define mmvgb_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3C800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgb_rr(XG, XS)                                                    \
        addgb3rr(W(XG), W(XG), W(XS))

#define addgb_ld(XG, MS, DS)                                                \
        addgb3ld(W(XG), W(XG), W(MS), W(DS))

#define addgb3rr(XD, XS, XT)                                                \
        EMITW(0x4E208400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E208400 | MXM(REG(XD), REG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgb_rr(XG, XS)                                                    \
        adsgb3rr(W(XG), W(XG), W(XS))

#define adsgb_ld(XG, MS, DS)                                                \
        adsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E200C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define adsgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E200C00 | MXM(REG(XD), REG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgc_rr(XG, XS)                                                    \
        adsgc3rr(W(XG), W(XG), W(XS))

#define adsgc_ld(XG, MS, DS)                                                \
        adsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgc3rr(XD, XS, XT)                                                \
        EMITW(0x4E200C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define adsgc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E200C00 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgb_rr(XG, XS)                                                    \
        subgb3rr(W(XG), W(XG), W(XS))

#define subgb_ld(XG, MS, DS)                                                \
        subgb3ld(W(XG), W(XG), W(MS), W(DS))

#define subgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E208400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E208400 | MXM(REG(XD), REG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgb_rr(XG, XS)                                                    \
        sbsgb3rr(W(XG), W(XG), W(XS))

#define sbsgb_ld(XG, MS, DS)                                                \
        sbsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E202C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define sbsgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E202C00 | MXM(REG(XD), REG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgc_rr(XG, XS)                                                    \
        sbsgc3rr(W(XG), W(XG), W(XS))

#define sbsgc_ld(XG, MS, DS)                                                \
        sbsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgc3rr(XD, XS, XT)                                                \
        EMITW(0x4E202C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define sbsgc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E202C00 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgb_rr(XG, XS)                                                    \
        mulgb3rr(W(XG), W(XG), W(XS))

#define mulgb_ld(XG, MS, DS)                                                \
        mulgb3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgb3rr(XD, XS, XT)                                                \
        EMITW(0x4E209C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E209C00 | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgb_ri(XG, IS)     /* emits shift-right with out-of-range args */ \
        shlgb3ri(W(XG), W(XG), W(IS))

#define shlgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgb3ri(XD, XS, IT)                                                \
        EMITW(0x4F080400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 8) & 0x00005000) | (M(VAL(IT) > 7) & 0x20000000) |     \
        (M(VAL(IT) < 8) & ((0x07 & VAL(IT)) << 16)))

#define shlgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E010400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgb_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrgb3ri(W(XG), W(XG), W(IS))

#define shrgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgb3ri(XD, XS, IT)                                                \
        EMITW(0x4F080400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))

#define shrgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E010400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgc_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrgc3ri(W(XG), W(XG), W(IS))

#define shrgc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgc3ri(XD, XS, IT)                                                \
        EMITW(0x4F080400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))

#define shrgc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E010400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E204400 | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgb3rr(W(XG), W(XG), W(XS))

#define svlgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), REG(XT)))

#define svlgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgb3rr(W(XG), W(XG), W(XS))

#define svrgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E20B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))

#define svrgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgc3rr(W(XG), W(XG), W(XS))

#define svrgc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgc3rr(XD, XS, XT)                                                \
        EMITW(0x6E20B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x4E204400 | MXM(REG(XD), REG(XS), TmmM))

#define svrgc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E204400 | MXM(REG(XD), REG(XS), TmmM))

/*****************   packed byte-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingb_rr(XG, XS)                                                    \
        mingb3rr(W(XG), W(XG), W(XS))

#define mingb_ld(XG, MS, DS)                                                \
        mingb3ld(W(XG), W(XG), W(MS), W(DS))

#define mingb3rr(XD, XS, XT)                                                \
        EMITW(0x6E206C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mingb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E206C00 | MXM(REG(XD), REG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingc_rr(XG, XS)                                                    \
        mingc3rr(W(XG), W(XG), W(XS))

#define mingc_ld(XG, MS, DS)                                                \
        mingc3ld(W(XG), W(XG), W(MS), W(DS))

#define mingc3rr(XD, XS, XT)                                                \
        EMITW(0x4E206C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mingc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E206C00 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgb_rr(XG, XS)                                                    \
        maxgb3rr(W(XG), W(XG), W(XS))

#define maxgb_ld(XG, MS, DS)                                                \
        maxgb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E206400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E206400 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgc_rr(XG, XS)                                                    \
        maxgc3rr(W(XG), W(XG), W(XS))

#define maxgc_ld(XG, MS, DS)                                                \
        maxgc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgc3rr(XD, XS, XT)                                                \
        EMITW(0x4E206400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxgc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E206400 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgb_rr(XG, XS)                                                    \
        ceqgb3rr(W(XG), W(XG), W(XS))

#define ceqgb_ld(XG, MS, DS)                                                \
        ceqgb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegb_rr(XG, XS)                                                    \
        cnegb3rr(W(XG), W(XG), W(XS))

#define cnegb_ld(XG, MS, DS)                                                \
        cnegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegb3rr(XD, XS, XT)                                                \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cnegb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgb_rr(XG, XS)                                                    \
        cltgb3rr(W(XG), W(XG), W(XS))

#define cltgb_ld(XG, MS, DS)                                                \
        cltgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E203400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E203400 | MXM(REG(XD), TmmM,    REG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgc_rr(XG, XS)                                                    \
        cltgc3rr(W(XG), W(XG), W(XS))

#define cltgc_ld(XG, MS, DS)                                                \
        cltgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgc3rr(XD, XS, XT)                                                \
        EMITW(0x4E203400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltgc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E203400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegb_rr(XG, XS)                                                    \
        clegb3rr(W(XG), W(XG), W(XS))

#define clegb_ld(XG, MS, DS)                                                \
        clegb3ld(W(XG), W(XG), W(MS), W(DS))

#define clegb3rr(XD, XS, XT)                                                \
        EMITW(0x6E203C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define clegb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E203C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegc_rr(XG, XS)                                                    \
        clegc3rr(W(XG), W(XG), W(XS))

#define clegc_ld(XG, MS, DS)                                                \
        clegc3ld(W(XG), W(XG), W(MS), W(DS))

#define clegc3rr(XD, XS, XT)                                                \
        EMITW(0x4E203C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define clegc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E203C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgb_rr(XG, XS)                                                    \
        cgtgb3rr(W(XG), W(XG), W(XS))

#define cgtgb_ld(XG, MS, DS)                                                \
        cgtgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgb3rr(XD, XS, XT)                                                \
        EMITW(0x6E203400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtgb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E203400 | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgc_rr(XG, XS)                                                    \
        cgtgc3rr(W(XG), W(XG), W(XS))

#define cgtgc_ld(XG, MS, DS)                                                \
        cgtgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgc3rr(XD, XS, XT)                                                \
        EMITW(0x4E203400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtgc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E203400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegb_rr(XG, XS)                                                    \
        cgegb3rr(W(XG), W(XG), W(XS))

#define cgegb_ld(XG, MS, DS)                                                \
        cgegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegb3rr(XD, XS, XT)                                                \
        EMITW(0x6E203C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgegb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E203C00 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegc_rr(XG, XS)                                                    \
        cgegc3rr(W(XG), W(XG), W(XS))

#define cgegc_ld(XG, MS, DS)                                                \
        cgegc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegc3rr(XD, XS, XT)                                                \
        EMITW(0x4E203C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgegc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E203C00 | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_AHB_128X1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

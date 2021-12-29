/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_128X1V8_H
#define RT_RTARCH_XHB_128X1V8_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_128x1v8.h: Implementation of x86_64 half+byte AVX1/2 instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdg*_rx - applies [cmd] to 128-bit packed SIMD: [r]egister (one operand)
 * cmdg*_rr - applies [cmd] to 128-bit packed SIMD: [r]egister from [r]egister
 *
 * cmdg*_rm - applies [cmd] to 128-bit packed SIMD: [r]egister from [m]emory
 * cmdg*_ld - applies [cmd] to 128-bit packed SIMD: as above (friendly alias)
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

#if (RT_128X1 >= 8 && RT_128X1 <= 32)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgx_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgx_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgx_rr(XG, XS)                                                    \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_rr(Xmm0, W(XG))                                               \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_rr(W(XG), Xmm0)

#define mmvgx_ld(XG, MS, DS)                                                \
        notgx_rx(Xmm0)                                                      \
        andgx_rr(W(XG), Xmm0)                                               \
        anngx_ld(Xmm0, W(MS), W(DS))                                        \
        orrgx_rr(W(XG), Xmm0)

#define mmvgx_st(XS, MG, DG)                                                \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_ld(Xmm0, W(MG), W(DG))                                        \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
        andgx3rr(W(XG), W(XG), W(XS))

#define andgx_ld(XG, MS, DS)                                                \
        andgx3ld(W(XG), W(XG), W(MS), W(DS))

#define andgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anngx_rr(XG, XS)                                                    \
        anngx3rr(W(XG), W(XG), W(XS))

#define anngx_ld(XG, MS, DS)                                                \
        anngx3ld(W(XG), W(XG), W(MS), W(DS))

#define anngx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anngx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrgx_rr(XG, XS)                                                    \
        orrgx3rr(W(XG), W(XG), W(XS))

#define orrgx_ld(XG, MS, DS)                                                \
        orrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orngx_rr(XG, XS)                                                    \
        notgx_rx(W(XG))                                                     \
        orrgx_rr(W(XG), W(XS))

#define orngx_ld(XG, MS, DS)                                                \
        notgx_rx(W(XG))                                                     \
        orrgx_ld(W(XG), W(MS), W(DS))

#define orngx3rr(XD, XS, XT)                                                \
        notgx_rr(W(XD), W(XS))                                              \
        orrgx_rr(W(XD), W(XT))

#define orngx3ld(XD, XS, MT, DT)                                            \
        notgx_rr(W(XD), W(XS))                                              \
        orrgx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorgx_rr(XG, XS)                                                    \
        xorgx3rr(W(XG), W(XG), W(XS))

#define xorgx_ld(XG, MS, DS)                                                \
        xorgx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        notgx_rr(W(XG), W(XG))

#define notgx_rr(XD, XS)                                                    \
        anngx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
        addgx3rr(W(XG), W(XG), W(XS))

#define addgx_ld(XG, MS, DS)                                                \
        addgx3ld(W(XG), W(XG), W(MS), W(DS))

#define addgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgx_rr(XG, XS)                                                    \
        adsgx3rr(W(XG), W(XG), W(XS))

#define adsgx_ld(XG, MS, DS)                                                \
        adsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgn_rr(XG, XS)                                                    \
        adsgn3rr(W(XG), W(XG), W(XS))

#define adsgn_ld(XG, MS, DS)                                                \
        adsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgn3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
        subgx3rr(W(XG), W(XG), W(XS))

#define subgx_ld(XG, MS, DS)                                                \
        subgx3ld(W(XG), W(XG), W(MS), W(DS))

#define subgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgx_rr(XG, XS)                                                    \
        sbsgx3rr(W(XG), W(XG), W(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
        sbsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgn_rr(XG, XS)                                                    \
        sbsgn3rr(W(XG), W(XG), W(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
        sbsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgn3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
        mulgx3rr(W(XG), W(XG), W(XS))

#define mulgx_ld(XG, MS, DS)                                                \
        mulgx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)                                                    \
        shlgx3ri(W(XG), W(XG), W(IS))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgx3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 0, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)                                                    \
        shrgx3ri(W(XG), W(XG), W(IS))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgx3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 0, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)                                                    \
        shrgn3ri(W(XG), W(XG), W(IS))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgn3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 0, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrgn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx3rr(W(XG), W(XG), W(XS))

#define svlgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgx_rx(W(XD))

#define svlgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlgx_rx(W(XD))

#define svlgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx3rr(W(XG), W(XG), W(XS))

#define svrgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgx_rx(W(XD))

#define svrgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgx_rx(W(XD))

#define svrgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn3rr(W(XG), W(XG), W(XS))

#define svrgn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgn3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgn_rx(W(XD))

#define svrgn3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgn_rx(W(XD))

#define svrgn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
        mingx3rr(W(XG), W(XG), W(XS))

#define mingx_ld(XG, MS, DS)                                                \
        mingx3ld(W(XG), W(XG), W(MS), W(DS))

#define mingx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingn_rr(XG, XS)                                                    \
        mingn3rr(W(XG), W(XG), W(XS))

#define mingn_ld(XG, MS, DS)                                                \
        mingn3ld(W(XG), W(XG), W(MS), W(DS))

#define mingn3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
        maxgx3rr(W(XG), W(XG), W(XS))

#define maxgx_ld(XG, MS, DS)                                                \
        maxgx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgn_rr(XG, XS)                                                    \
        maxgn3rr(W(XG), W(XG), W(XS))

#define maxgn_ld(XG, MS, DS)                                                \
        maxgn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgn3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgx_rr(XG, XS)                                                    \
        ceqgx3rr(W(XG), W(XG), W(XS))

#define ceqgx_ld(XG, MS, DS)                                                \
        ceqgx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqgx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegx_rr(XG, XS)                                                    \
        cnegx3rr(W(XG), W(XG), W(XS))

#define cnegx_ld(XG, MS, DS)                                                \
        cnegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegx3rr(XD, XS, XT)                                                \
        ceqgx3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define cnegx3ld(XD, XS, MT, DT)                                            \
        ceqgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgx_rr(XG, XS)                                                    \
        cltgx3rr(W(XG), W(XG), W(XS))

#define cltgx_ld(XG, MS, DS)                                                \
        cltgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgx3rr(XD, XS, XT)                                                \
        mingx3rr(W(XD), W(XS), W(XT))                                       \
        cnegx_rr(W(XD), W(XT))

#define cltgx3ld(XD, XS, MT, DT)                                            \
        mingx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgn_rr(XG, XS)                                                    \
        cltgn3rr(W(XG), W(XG), W(XS))

#define cltgn_ld(XG, MS, DS)                                                \
        cltgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgn3rr(XD, XS, XT)                                                \
        cgtgn3rr(W(XD), W(XT), W(XS))

#define cltgn3ld(XD, XS, MT, DT)                                            \
        mingn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegx_rr(XG, XS)                                                    \
        clegx3rr(W(XG), W(XG), W(XS))

#define clegx_ld(XG, MS, DS)                                                \
        clegx3ld(W(XG), W(XG), W(MS), W(DS))

#define clegx3rr(XD, XS, XT)                                                \
        maxgx3rr(W(XD), W(XS), W(XT))                                       \
        ceqgx_rr(W(XD), W(XT))

#define clegx3ld(XD, XS, MT, DT)                                            \
        maxgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegn_rr(XG, XS)                                                    \
        clegn3rr(W(XG), W(XG), W(XS))

#define clegn_ld(XG, MS, DS)                                                \
        clegn3ld(W(XG), W(XG), W(MS), W(DS))

#define clegn3rr(XD, XS, XT)                                                \
        cgtgn3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define clegn3ld(XD, XS, MT, DT)                                            \
        cgtgn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgx_rr(XG, XS)                                                    \
        cgtgx3rr(W(XG), W(XG), W(XS))

#define cgtgx_ld(XG, MS, DS)                                                \
        cgtgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgx3rr(XD, XS, XT)                                                \
        maxgx3rr(W(XD), W(XS), W(XT))                                       \
        cnegx_rr(W(XD), W(XT))

#define cgtgx3ld(XD, XS, MT, DT)                                            \
        maxgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegx_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgn_rr(XG, XS)                                                    \
        cgtgn3rr(W(XG), W(XG), W(XS))

#define cgtgn_ld(XG, MS, DS)                                                \
        cgtgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgn3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtgn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegx_rr(XG, XS)                                                    \
        cgegx3rr(W(XG), W(XG), W(XS))

#define cgegx_ld(XG, MS, DS)                                                \
        cgegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegx3rr(XD, XS, XT)                                                \
        mingx3rr(W(XD), W(XS), W(XT))                                       \
        ceqgx_rr(W(XD), W(XT))

#define cgegx3ld(XD, XS, MT, DT)                                            \
        mingx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegn_rr(XG, XS)                                                    \
        cgegn3rr(W(XG), W(XG), W(XS))

#define cgegn_ld(XG, MS, DS)                                                \
        cgegn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegn3rr(XD, XS, XT)                                                \
        mingn3rr(W(XD), W(XS), W(XT))                                       \
        ceqgx_rr(W(XD), W(XT))

#define cgegn3ld(XD, XS, MT, DT)                                            \
        mingn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgx_ld(W(XD), W(MT), W(DT))

/****************   packed byte-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgb_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgb_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgb_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgb_rr(XG, XS)                                                    \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_rr(Xmm0, W(XG))                                               \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgb_rr(W(XG), Xmm0)

#define mmvgb_ld(XG, MS, DS)                                                \
        notgx_rx(Xmm0)                                                      \
        andgx_rr(W(XG), Xmm0)                                               \
        anngx_ld(Xmm0, W(MS), W(DS))                                        \
        orrgx_rr(W(XG), Xmm0)

#define mmvgb_st(XS, MG, DG)                                                \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_ld(Xmm0, W(MG), W(DG))                                        \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgb_st(Xmm0, W(MG), W(DG))

/* logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgb_rr(XG, XS)                                                    \
        addgb3rr(W(XG), W(XG), W(XS))

#define addgb_ld(XG, MS, DS)                                                \
        addgb3ld(W(XG), W(XG), W(MS), W(DS))

#define addgb3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addgb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgb_rr(XG, XS)                                                    \
        adsgb3rr(W(XG), W(XG), W(XS))

#define adsgb_ld(XG, MS, DS)                                                \
        adsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgb3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgc_rr(XG, XS)                                                    \
        adsgc3rr(W(XG), W(XG), W(XS))

#define adsgc_ld(XG, MS, DS)                                                \
        adsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgc3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgb_rr(XG, XS)                                                    \
        subgb3rr(W(XG), W(XG), W(XS))

#define subgb_ld(XG, MS, DS)                                                \
        subgb3ld(W(XG), W(XG), W(MS), W(DS))

#define subgb3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subgb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgb_rr(XG, XS)                                                    \
        sbsgb3rr(W(XG), W(XG), W(XS))

#define sbsgb_ld(XG, MS, DS)                                                \
        sbsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgb3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgc_rr(XG, XS)                                                    \
        sbsgc3rr(W(XG), W(XG), W(XS))

#define sbsgc_ld(XG, MS, DS)                                                \
        sbsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgc3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgb_rr(XG, XS)                                                    \
        mulgb3rr(W(XG), W(XG), W(XS))

#define mulgb_ld(XG, MS, DS)                                                \
        mulgb3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgb3rr(XD, XS, XT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulgb_rx(W(XD))

#define mulgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulgb_rx(W(XD))

#define mulgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x01))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x01))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x02))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x02))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x03))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x03))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x05))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x05))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x06))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x06))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x07))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x07))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x09))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x09))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0F))                              \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgb_ri(XG, IS)                                                    \
        shlgb3ri(W(XG), W(XG), W(IS))

#define shlgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgb3ri(XD, XS, IT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgb_xx() /* not portable, do not use outside */                   \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgb_ri(XG, IS)                                                    \
        shrgb3ri(W(XG), W(XG), W(IS))

#define shrgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgb3ri(XD, XS, IT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgb_xx() /* not portable, do not use outside */                   \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgc_ri(XG, IS)                                                    \
        shrgc3ri(W(XG), W(XG), W(IS))

#define shrgc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgc3ri(XD, XS, IT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgc3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgc_xx() /* not portable, do not use outside */                   \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgb3rr(W(XG), W(XG), W(XS))

#define svlgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgb3rr(XD, XS, XT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgb_rx(W(XD))

#define svlgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlgb_rx(W(XD))

#define svlgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgb3rr(W(XG), W(XG), W(XS))

#define svrgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgb3rr(XD, XS, XT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgb_rx(W(XD))

#define svrgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgb_rx(W(XD))

#define svrgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgc3rr(W(XG), W(XG), W(XS))

#define svrgc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgc3rr(XD, XS, XT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgc_rx(W(XD))

#define svrgc3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgc_rx(W(XD))

#define svrgc_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingb_rr(XG, XS)                                                    \
        mingb3rr(W(XG), W(XG), W(XS))

#define mingb_ld(XG, MS, DS)                                                \
        mingb3ld(W(XG), W(XG), W(MS), W(DS))

#define mingb3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingc_rr(XG, XS)                                                    \
        mingc3rr(W(XG), W(XG), W(XS))

#define mingc_ld(XG, MS, DS)                                                \
        mingc3ld(W(XG), W(XG), W(MS), W(DS))

#define mingc3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgb_rr(XG, XS)                                                    \
        maxgb3rr(W(XG), W(XG), W(XS))

#define maxgb_ld(XG, MS, DS)                                                \
        maxgb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgb3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgc_rr(XG, XS)                                                    \
        maxgc3rr(W(XG), W(XG), W(XS))

#define maxgc_ld(XG, MS, DS)                                                \
        maxgc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgc3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgb_rr(XG, XS)                                                    \
        ceqgb3rr(W(XG), W(XG), W(XS))

#define ceqgb_ld(XG, MS, DS)                                                \
        ceqgb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgb3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqgb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegb_rr(XG, XS)                                                    \
        cnegb3rr(W(XG), W(XG), W(XS))

#define cnegb_ld(XG, MS, DS)                                                \
        cnegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegb3rr(XD, XS, XT)                                                \
        ceqgb3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define cnegb3ld(XD, XS, MT, DT)                                            \
        ceqgb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgb_rr(XG, XS)                                                    \
        cltgb3rr(W(XG), W(XG), W(XS))

#define cltgb_ld(XG, MS, DS)                                                \
        cltgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgb3rr(XD, XS, XT)                                                \
        mingb3rr(W(XD), W(XS), W(XT))                                       \
        cnegb_rr(W(XD), W(XT))

#define cltgb3ld(XD, XS, MT, DT)                                            \
        mingb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegb_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgc_rr(XG, XS)                                                    \
        cltgc3rr(W(XG), W(XG), W(XS))

#define cltgc_ld(XG, MS, DS)                                                \
        cltgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgc3rr(XD, XS, XT)                                                \
        cgtgc3rr(W(XD), W(XT), W(XS))

#define cltgc3ld(XD, XS, MT, DT)                                            \
        mingc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegb_rr(XG, XS)                                                    \
        clegb3rr(W(XG), W(XG), W(XS))

#define clegb_ld(XG, MS, DS)                                                \
        clegb3ld(W(XG), W(XG), W(MS), W(DS))

#define clegb3rr(XD, XS, XT)                                                \
        maxgb3rr(W(XD), W(XS), W(XT))                                       \
        ceqgb_rr(W(XD), W(XT))

#define clegb3ld(XD, XS, MT, DT)                                            \
        maxgb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegc_rr(XG, XS)                                                    \
        clegc3rr(W(XG), W(XG), W(XS))

#define clegc_ld(XG, MS, DS)                                                \
        clegc3ld(W(XG), W(XG), W(MS), W(DS))

#define clegc3rr(XD, XS, XT)                                                \
        cgtgc3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define clegc3ld(XD, XS, MT, DT)                                            \
        cgtgc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgb_rr(XG, XS)                                                    \
        cgtgb3rr(W(XG), W(XG), W(XS))

#define cgtgb_ld(XG, MS, DS)                                                \
        cgtgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgb3rr(XD, XS, XT)                                                \
        maxgb3rr(W(XD), W(XS), W(XT))                                       \
        cnegb_rr(W(XD), W(XT))

#define cgtgb3ld(XD, XS, MT, DT)                                            \
        maxgb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegb_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgc_rr(XG, XS)                                                    \
        cgtgc3rr(W(XG), W(XG), W(XS))

#define cgtgc_ld(XG, MS, DS)                                                \
        cgtgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgc3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtgc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegb_rr(XG, XS)                                                    \
        cgegb3rr(W(XG), W(XG), W(XS))

#define cgegb_ld(XG, MS, DS)                                                \
        cgegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegb3rr(XD, XS, XT)                                                \
        mingb3rr(W(XD), W(XS), W(XT))                                       \
        ceqgb_rr(W(XD), W(XT))

#define cgegb3ld(XD, XS, MT, DT)                                            \
        mingb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgb_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegc_rr(XG, XS)                                                    \
        cgegc3rr(W(XG), W(XG), W(XS))

#define cgegc_ld(XG, MS, DS)                                                \
        cgegc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegc3rr(XD, XS, XT)                                                \
        mingc3rr(W(XD), W(XS), W(XT))                                       \
        ceqgb_rr(W(XD), W(XT))

#define cgegc3ld(XD, XS, MT, DT)                                            \
        mingc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgb_ld(W(XD), W(MT), W(DT))

/******* instructions below are only available on CPUs with AVX512/fp16 *******/

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/**********   scalar half-precision floating-point move/arithmetic   **********/

/* mov (D = S) */

#define movns_rr(XD, XS)                                                    \
        E5X(RXB(XD), RXB(XS), REN(XD), 0, 2, 1) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movns_ld(XD, MS, DS)                                                \
    ADR E5X(RXB(XD), RXB(MS),    0x00, 0, 2, 1) EMITB(0x10)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movns_st(XS, MD, DD)                                                \
    ADR E5X(RXB(XS), RXB(MD),    0x00, 0, 2, 1) EMITB(0x11)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addns_rr(XG, XS)                                                    \
        addns3rr(W(XG), W(XG), W(XS))

#define addns_ld(XG, MS, DS)                                                \
        addns3ld(W(XG), W(XG), W(MS), W(DS))

#define addns3rr(XD, XS, XT)                                                \
        E5X(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addns3ld(XD, XS, MT, DT)                                            \
    ADR E5X(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subns_rr(XG, XS)                                                    \
        subns3rr(W(XG), W(XG), W(XS))

#define subns_ld(XG, MS, DS)                                                \
        subns3ld(W(XG), W(XG), W(MS), W(DS))

#define subns3rr(XD, XS, XT)                                                \
        E5X(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subns3ld(XD, XS, MT, DT)                                            \
    ADR E5X(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulns_rr(XG, XS)                                                    \
        mulns3rr(W(XG), W(XG), W(XS))

#define mulns_ld(XG, MS, DS)                                                \
        mulns3ld(W(XG), W(XG), W(MS), W(DS))

#define mulns3rr(XD, XS, XT)                                                \
        E5X(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulns3ld(XD, XS, MT, DT)                                            \
    ADR E5X(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divns_rr(XG, XS)                                                    \
        divns3rr(W(XG), W(XG), W(XS))

#define divns_ld(XG, MS, DS)                                                \
        divns3ld(W(XG), W(XG), W(MS), W(DS))

#define divns3rr(XD, XS, XT)                                                \
        E5X(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divns3ld(XD, XS, MT, DT)                                            \
    ADR E5X(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqrns_rr(XD, XS)                                                    \
        E5X(RXB(XD), RXB(XS),    0x00, 0, 2, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrns_ld(XD, MS, DS)                                                \
    ADR E5X(RXB(XD), RXB(MS),    0x00, 0, 2, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcens_rr(XD, XS)                                                    \
        E5X(RXB(XD), RXB(XS),    0x00, 0, 1, 2) EMITB(0x4D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsns_rr(XG, XS) /* destroys XS */                                  \
        mulns_rr(W(XS), W(XG))                                              \
        mulns_rr(W(XS), W(XG))                                              \
        addns_rr(W(XG), W(XG))                                              \
        subns_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsens_rr(XD, XS)                                                    \
        E5X(RXB(XD), RXB(XS),    0x00, 0, 1, 2) EMITB(0x4F)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmans_rr(XG, XS, XT)                                                \
    ADR E5X(RXB(XG), RXB(XT), REN(XS), 0, 1, 2) EMITB(0xB9)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmans_ld(XG, XS, MT, DT)                                            \
    ADR E5X(RXB(XG), RXB(MT), REN(XS), 0, 1, 2) EMITB(0xB9)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsns_rr(XG, XS, XT)                                                \
    ADR E5X(RXB(XG), RXB(XT), REN(XS), 0, 1, 2) EMITB(0xBD)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsns_ld(XG, XS, MT, DT)                                            \
    ADR E5X(RXB(XG), RXB(MT), REN(XS), 0, 1, 2) EMITB(0xBD)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/**************   scalar half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minns_rr(XG, XS)                                                    \
        minns3rr(W(XG), W(XG), W(XS))

#define minns_ld(XG, MS, DS)                                                \
        minns3ld(W(XG), W(XG), W(MS), W(DS))

#define minns3rr(XD, XS, XT)                                                \
        E5X(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minns3ld(XD, XS, MT, DT)                                            \
    ADR E5X(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxns_rr(XG, XS)                                                    \
        maxns3rr(W(XG), W(XG), W(XS))

#define maxns_ld(XG, MS, DS)                                                \
        maxns3ld(W(XG), W(XG), W(MS), W(DS))

#define maxns3rr(XD, XS, XT)                                                \
        E5X(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxns3ld(XD, XS, MT, DT)                                            \
    ADR E5X(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqns_rr(XG, XS)                                                    \
        ceqns3rr(W(XG), W(XG), W(XS))

#define ceqns_ld(XG, MS, DS)                                                \
        ceqns3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqns3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

#define ceqns3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnens_rr(XG, XS)                                                    \
        cnens3rr(W(XG), W(XG), W(XS))

#define cnens_ld(XG, MS, DS)                                                \
        cnens3ld(W(XG), W(XG), W(MS), W(DS))

#define cnens3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

#define cnens3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltns_rr(XG, XS)                                                    \
        cltns3rr(W(XG), W(XG), W(XS))

#define cltns_ld(XG, MS, DS)                                                \
        cltns3ld(W(XG), W(XG), W(MS), W(DS))

#define cltns3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

#define cltns3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clens_rr(XG, XS)                                                    \
        clens3rr(W(XG), W(XG), W(XS))

#define clens_ld(XG, MS, DS)                                                \
        clens3ld(W(XG), W(XG), W(MS), W(DS))

#define clens3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

#define clens3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtns_rr(XG, XS)                                                    \
        cgtns3rr(W(XG), W(XG), W(XS))

#define cgtns_ld(XG, MS, DS)                                                \
        cgtns3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtns3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

#define cgtns3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgens_rr(XG, XS)                                                    \
        cgens3rr(W(XG), W(XG), W(XS))

#define cgens_ld(XG, MS, DS)                                                \
        cgens3ld(W(XG), W(XG), W(MS), W(DS))

#define cgens3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)

#define cgens3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 2, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1nx_ld(W(XD), Mebp, inf_GPC07)


#define mz1nx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR EZX(RXB(XG), RXB(MS),    0x00, 0, 2, 1) EMITB(0x10)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_128X1V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_128X1V4_H
#define RT_RTARCH_XHB_128X1V4_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_128x1v4.h: Implementation of x86_64 half+byte SSE2/4 instructions.
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

#if (RT_128X1 == 4)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgx_ld(XD, MS, DS)                                                \
    ADR REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgx_st(XS, MD, DD)                                                \
    ADR REX(RXB(XS), RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
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
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        andgx_rr(W(XD), W(XT))

#define andgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        andgx_ld(W(XD), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anngx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define anngx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define anngx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        anngx_rr(W(XD), W(XT))

#define anngx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        anngx_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        orrgx_rr(W(XD), W(XT))

#define orrgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        orrgx_ld(W(XD), W(MT), W(DT))

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
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_rr(W(XD), W(XT))

#define xorgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_ld(W(XD), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        anngx_ld(W(XG), Mebp, inf_GPC07)

#define notgx_rr(XD, XS)                                                    \
        movgx_rr(W(XD), W(XS))                                              \
        notgx_rx(W(XD))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        addgx_rr(W(XD), W(XT))

#define addgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        addgx_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define adsgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        adsgx_rr(W(XD), W(XT))

#define adsgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        adsgx_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define adsgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        adsgn_rr(W(XD), W(XT))

#define adsgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        adsgn_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        subgx_rr(W(XD), W(XT))

#define subgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        subgx_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbsgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgx_rr(W(XD), W(XT))

#define sbsgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgx_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbsgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgn_rr(W(XD), W(XT))

#define sbsgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgn_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mulgx_rr(W(XD), W(XT))

#define mulgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mulgx_ld(W(XD), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xF1)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shlgx3ri(XD, XS, IT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        shlgx_ri(W(XD), W(IT))

#define shlgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        shlgx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD1)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrgx3ri(XD, XS, IT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        shrgx_ri(W(XD), W(IT))

#define shrgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        shrgx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xE1)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrgn3ri(XD, XS, IT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        shrgn_ri(W(XD), W(IT))

#define shrgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        shrgn_ld(W(XD), W(MT), W(DT))

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

#if (RT_SIMD_COMPAT_SSE < 4)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
        mingx3rr(W(XG), W(XG), W(XS))

#define mingx_ld(XG, MS, DS)                                                \
        mingx3ld(W(XG), W(XG), W(MS), W(DS))

#define mingx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mingx_rx(W(XD))

#define mingx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mingx_rx(W(XD))

#define mingx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        stack_ld(Reax)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR02(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
        maxgx3rr(W(XG), W(XG), W(XS))

#define maxgx_ld(XG, MS, DS)                                                \
        maxgx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxgx_rx(W(XD))

#define maxgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxgx_rx(W(XD))

#define maxgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        stack_ld(Reax)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR02(0))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mingx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mingx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mingx_rr(W(XD), W(XT))

#define mingx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mingx_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        maxgx_rr(W(XD), W(XT))

#define maxgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        maxgx_ld(W(XD), W(MT), W(DT))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mingn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mingn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mingn_rr(W(XD), W(XT))

#define mingn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mingn_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        maxgn_rr(W(XD), W(XT))

#define maxgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        maxgn_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define ceqgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define ceqgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        ceqgx_rr(W(XD), W(XT))

#define ceqgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        ceqgx_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegx_rr(XG, XS)                                                    \
        ceqgx_rr(W(XG), W(XS))                                              \
        notgx_rx(W(XG))

#define cnegx_ld(XG, MS, DS)                                                \
        ceqgx_ld(W(XG), W(MS), W(DS))                                       \
        notgx_rx(W(XG))

#define cnegx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cnegx_rr(W(XD), W(XT))

#define cnegx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cnegx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgx_rr(XG, XS)                                                    \
        mingx_rr(W(XG), W(XS))                                              \
        cnegx_rr(W(XG), W(XS))

#define cltgx_ld(XG, MS, DS)                                                \
        mingx_ld(W(XG), W(MS), W(DS))                                       \
        cnegx_ld(W(XG), W(MS), W(DS))

#define cltgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cltgx_rr(W(XD), W(XT))

#define cltgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cltgx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgn_rr(XG, XS)                                                    \
        mingn_rr(W(XG), W(XS))                                              \
        cnegx_rr(W(XG), W(XS))

#define cltgn_ld(XG, MS, DS)                                                \
        mingn_ld(W(XG), W(MS), W(DS))                                       \
        cnegx_ld(W(XG), W(MS), W(DS))

#define cltgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cltgn_rr(W(XD), W(XT))

#define cltgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cltgn_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegx_rr(XG, XS)                                                    \
        maxgx_rr(W(XG), W(XS))                                              \
        ceqgx_rr(W(XG), W(XS))

#define clegx_ld(XG, MS, DS)                                                \
        maxgx_ld(W(XG), W(MS), W(DS))                                       \
        ceqgx_ld(W(XG), W(MS), W(DS))

#define clegx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        clegx_rr(W(XD), W(XT))

#define clegx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        clegx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegn_rr(XG, XS)                                                    \
        cgtgn_rr(W(XG), W(XS))                                              \
        notgx_rx(W(XG))

#define clegn_ld(XG, MS, DS)                                                \
        cgtgn_ld(W(XG), W(MS), W(DS))                                       \
        notgx_rx(W(XG))

#define clegn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        clegn_rr(W(XD), W(XT))

#define clegn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        clegn_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgx_rr(XG, XS)                                                    \
        maxgx_rr(W(XG), W(XS))                                              \
        cnegx_rr(W(XG), W(XS))

#define cgtgx_ld(XG, MS, DS)                                                \
        maxgx_ld(W(XG), W(MS), W(DS))                                       \
        cnegx_ld(W(XG), W(MS), W(DS))

#define cgtgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgx_rr(W(XD), W(XT))

#define cgtgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgx_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cgtgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cgtgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgn_rr(W(XD), W(XT))

#define cgtgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgn_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegx_rr(XG, XS)                                                    \
        mingx_rr(W(XG), W(XS))                                              \
        ceqgx_rr(W(XG), W(XS))

#define cgegx_ld(XG, MS, DS)                                                \
        mingx_ld(W(XG), W(MS), W(DS))                                       \
        ceqgx_ld(W(XG), W(MS), W(DS))

#define cgegx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgegx_rr(W(XD), W(XT))

#define cgegx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgegx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegn_rr(XG, XS)                                                    \
        mingn_rr(W(XG), W(XS))                                              \
        ceqgx_rr(W(XG), W(XS))

#define cgegn_ld(XG, MS, DS)                                                \
        mingn_ld(W(XG), W(MS), W(DS))                                       \
        ceqgx_ld(W(XG), W(MS), W(DS))

#define cgegn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgegn_rr(W(XD), W(XT))

#define cgegn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgegn_ld(W(XD), W(MT), W(DT))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_128X1V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_128X2V4_H
#define RT_RTARCH_XHB_128X2V4_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_128x2v4.h: Implementation of x86_64 half+byte SSE2/4 pairs.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmda*_rx - applies [cmd] to 256-bit packed SIMD: [r]egister (one operand)
 * cmda*_rr - applies [cmd] to 256-bit packed SIMD: [r]egister from [r]egister
 *
 * cmda*_rm - applies [cmd] to 256-bit packed SIMD: [r]egister from [m]emory
 * cmda*_ld - applies [cmd] to 256-bit packed SIMD: as above (friendly alias)
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

#if (RT_128X2 == 4)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movax_rr(XD, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movax_ld(XD, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define movax_st(XS, MD, DD)                                                \
    ADR REX(0,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR REX(1,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VYL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvax_rr(XG, XS)                                                    \
        andax_rr(W(XS), Xmm0)                                               \
        annax_rr(Xmm0, W(XG))                                               \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_rr(W(XG), Xmm0)

#define mmvax_ld(XG, MS, DS)                                                \
        notax_rx(Xmm0)                                                      \
        andax_rr(W(XG), Xmm0)                                               \
        annax_ld(Xmm0, W(MS), W(DS))                                        \
        orrax_rr(W(XG), Xmm0)

#define mmvax_st(XS, MG, DG)                                                \
        andax_rr(W(XS), Xmm0)                                               \
        annax_ld(Xmm0, W(MG), W(DG))                                        \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define andax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        andax_rr(W(XD), W(XT))

#define andax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        andax_ld(W(XD), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define annax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        annax_rr(W(XD), W(XT))

#define annax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        annax_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define orrax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        orrax_rr(W(XD), W(XT))

#define orrax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        orrax_ld(W(XD), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornax_rr(XG, XS)                                                    \
        notax_rx(W(XG))                                                     \
        orrax_rr(W(XG), W(XS))

#define ornax_ld(XG, MS, DS)                                                \
        notax_rx(W(XG))                                                     \
        orrax_ld(W(XG), W(MS), W(DS))

#define ornax3rr(XD, XS, XT)                                                \
        notax_rr(W(XD), W(XS))                                              \
        orrax_rr(W(XD), W(XT))

#define ornax3ld(XD, XS, MT, DT)                                            \
        notax_rr(W(XD), W(XS))                                              \
        orrax_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define xorax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        xorax_rr(W(XD), W(XT))

#define xorax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        xorax_ld(W(XD), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notax_rx(XG)                                                        \
        annax_ld(W(XG), Mebp, inf_GPC07)

#define notax_rr(XD, XS)                                                    \
        movax_rr(W(XD), W(XS))                                              \
        notax_rx(W(XD))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define addax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        addax_rr(W(XD), W(XT))

#define addax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        addax_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define adsax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        adsax_rr(W(XD), W(XT))

#define adsax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        adsax_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsan_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define adsan3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        adsan_rr(W(XD), W(XT))

#define adsan3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        adsan_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define subax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        subax_rr(W(XD), W(XT))

#define subax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        subax_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define sbsax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        sbsax_rr(W(XD), W(XT))

#define sbsax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        sbsax_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsan_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define sbsan3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        sbsan_rr(W(XD), W(XT))

#define sbsan3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        sbsan_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define mulax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        mulax_rr(W(XD), W(XT))

#define mulax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        mulax_ld(W(XD), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))                               \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xF1)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xF1)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define shlax3ri(XD, XS, IT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        shlax_ri(W(XD), W(IT))

#define shlax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        shlax_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))                               \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xD1)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xD1)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define shrax3ri(XD, XS, IT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        shrax_ri(W(XD), W(IT))

#define shrax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        shrax_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))                               \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xE1)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xE1)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define shran3ri(XD, XS, IT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        shran_ri(W(XD), W(IT))

#define shran3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        shran_ld(W(XD), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlax3rr(W(XG), W(XG), W(XS))

#define svlax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlax3ld(W(XG), W(XG), W(MS), W(DS))

#define svlax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlax_rx(W(XD))

#define svlax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlax_rx(W(XD))

#define svlax_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrax3rr(W(XG), W(XG), W(XS))

#define svrax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrax3ld(W(XG), W(XG), W(MS), W(DS))

#define svrax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrax_rx(W(XD))

#define svrax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrax_rx(W(XD))

#define svrax_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svran_rr(XG, XS)     /* variable shift with per-elem count */       \
        svran3rr(W(XG), W(XG), W(XS))

#define svran_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svran3ld(W(XG), W(XG), W(MS), W(DS))

#define svran3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svran_rx(W(XD))

#define svran3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svran_rx(W(XD))

#define svran_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed half-precision integer compare   ******************/

#if (RT_SIMD_COMPAT_SSE < 4)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minax_rr(XG, XS)                                                    \
        minax3rr(W(XG), W(XG), W(XS))

#define minax_ld(XG, MS, DS)                                                \
        minax3ld(W(XG), W(XG), W(MS), W(DS))

#define minax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minax_rx(W(XD))

#define minax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minax_rx(W(XD))

#define minax_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x12))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x12))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x12))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x14))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x14))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x14))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x16))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x16))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x16))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x1A))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x1A))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x1A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x1C))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x1C))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x1C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x1E))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x1E))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x1E))                              \
        stack_ld(Reax)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR02(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxax_rr(XG, XS)                                                    \
        maxax3rr(W(XG), W(XG), W(XS))

#define maxax_ld(XG, MS, DS)                                                \
        maxax3ld(W(XG), W(XG), W(MS), W(DS))

#define maxax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxax_rx(W(XD))

#define maxax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxax_rx(W(XD))

#define maxax_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x12))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x12))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x12))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x14))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x14))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x14))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x16))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x16))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x16))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x1A))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x1A))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x1A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x1C))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x1C))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x1C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x1E))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x1E))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x1E))                              \
        stack_ld(Reax)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR02(0))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define minax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        minax_rr(W(XD), W(XT))

#define minax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        minax_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define maxax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        maxax_rr(W(XD), W(XT))

#define maxax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        maxax_ld(W(XD), W(MT), W(DT))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minan_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minan_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define minan3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        minan_rr(W(XD), W(XT))

#define minan3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        minan_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxan_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxan_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define maxan3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        maxan_rr(W(XD), W(XT))

#define maxan3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        maxan_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqax_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define ceqax_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define ceqax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        ceqax_rr(W(XD), W(XT))

#define ceqax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        ceqax_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneax_rr(XG, XS)                                                    \
        ceqax_rr(W(XG), W(XS))                                              \
        notax_rx(W(XG))

#define cneax_ld(XG, MS, DS)                                                \
        ceqax_ld(W(XG), W(MS), W(DS))                                       \
        notax_rx(W(XG))

#define cneax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cneax_rr(W(XD), W(XT))

#define cneax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cneax_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltax_rr(XG, XS)                                                    \
        minax_rr(W(XG), W(XS))                                              \
        cneax_rr(W(XG), W(XS))

#define cltax_ld(XG, MS, DS)                                                \
        minax_ld(W(XG), W(MS), W(DS))                                       \
        cneax_ld(W(XG), W(MS), W(DS))

#define cltax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cltax_rr(W(XD), W(XT))

#define cltax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cltax_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltan_rr(XG, XS)                                                    \
        minan_rr(W(XG), W(XS))                                              \
        cneax_rr(W(XG), W(XS))

#define cltan_ld(XG, MS, DS)                                                \
        minan_ld(W(XG), W(MS), W(DS))                                       \
        cneax_ld(W(XG), W(MS), W(DS))

#define cltan3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cltan_rr(W(XD), W(XT))

#define cltan3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cltan_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleax_rr(XG, XS)                                                    \
        maxax_rr(W(XG), W(XS))                                              \
        ceqax_rr(W(XG), W(XS))

#define cleax_ld(XG, MS, DS)                                                \
        maxax_ld(W(XG), W(MS), W(DS))                                       \
        ceqax_ld(W(XG), W(MS), W(DS))

#define cleax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cleax_rr(W(XD), W(XT))

#define cleax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cleax_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clean_rr(XG, XS)                                                    \
        cgtan_rr(W(XG), W(XS))                                              \
        notax_rx(W(XG))

#define clean_ld(XG, MS, DS)                                                \
        cgtan_ld(W(XG), W(MS), W(DS))                                       \
        notax_rx(W(XG))

#define clean3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        clean_rr(W(XD), W(XT))

#define clean3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        clean_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtax_rr(XG, XS)                                                    \
        maxax_rr(W(XG), W(XS))                                              \
        cneax_rr(W(XG), W(XS))

#define cgtax_ld(XG, MS, DS)                                                \
        maxax_ld(W(XG), W(MS), W(DS))                                       \
        cneax_ld(W(XG), W(MS), W(DS))

#define cgtax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cgtax_rr(W(XD), W(XT))

#define cgtax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cgtax_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtan_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cgtan_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define cgtan3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cgtan_rr(W(XD), W(XT))

#define cgtan3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cgtan_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeax_rr(XG, XS)                                                    \
        minax_rr(W(XG), W(XS))                                              \
        ceqax_rr(W(XG), W(XS))

#define cgeax_ld(XG, MS, DS)                                                \
        minax_ld(W(XG), W(MS), W(DS))                                       \
        ceqax_ld(W(XG), W(MS), W(DS))

#define cgeax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cgeax_rr(W(XD), W(XT))

#define cgeax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cgeax_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgean_rr(XG, XS)                                                    \
        minan_rr(W(XG), W(XS))                                              \
        ceqax_rr(W(XG), W(XS))

#define cgean_ld(XG, MS, DS)                                                \
        minan_ld(W(XG), W(MS), W(DS))                                       \
        ceqax_ld(W(XG), W(MS), W(DS))

#define cgean3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        cgean_rr(W(XD), W(XT))

#define cgean3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        cgean_ld(W(XD), W(MT), W(DT))

/****************   packed byte-precision generic move/logic   ****************/

/* mov (D = S) */

#define movab_rr(XD, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movab_ld(XD, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define movab_st(XS, MD, DD)                                                \
    ADR REX(0,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR REX(1,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VYL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvab_rr(XG, XS)                                                    \
        andax_rr(W(XS), Xmm0)                                               \
        annax_rr(Xmm0, W(XG))                                               \
        orrax_rr(Xmm0, W(XS))                                               \
        movab_rr(W(XG), Xmm0)

#define mmvab_ld(XG, MS, DS)                                                \
        notax_rx(Xmm0)                                                      \
        andax_rr(W(XG), Xmm0)                                               \
        annax_ld(Xmm0, W(MS), W(DS))                                        \
        orrax_rr(W(XG), Xmm0)

#define mmvab_st(XS, MG, DG)                                                \
        andax_rr(W(XS), Xmm0)                                               \
        annax_ld(Xmm0, W(MG), W(DG))                                        \
        orrax_rr(Xmm0, W(XS))                                               \
        movab_st(Xmm0, W(MG), W(DG))

/* logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addab_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xFC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xFC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addab_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xFC)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xFC)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define addab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        addab_rr(W(XD), W(XT))

#define addab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        addab_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsab_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xDC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xDC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsab_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xDC)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xDC)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define adsab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        adsab_rr(W(XD), W(XT))

#define adsab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        adsab_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsac_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xEC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xEC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsac_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xEC)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xEC)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define adsac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        adsac_rr(W(XD), W(XT))

#define adsac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        adsac_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subab_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xF8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xF8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subab_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xF8)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xF8)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define subab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        subab_rr(W(XD), W(XT))

#define subab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        subab_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsab_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xD8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xD8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsab_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xD8)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xD8)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define sbsab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        sbsab_rr(W(XD), W(XT))

#define sbsab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        sbsab_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsac_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xE8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xE8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsac_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xE8)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xE8)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define sbsac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        sbsac_rr(W(XD), W(XT))

#define sbsac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        sbsac_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulab_rr(XG, XS)                                                    \
        mulab3rr(W(XG), W(XG), W(XS))

#define mulab_ld(XG, MS, DS)                                                \
        mulab3ld(W(XG), W(XG), W(MS), W(DS))

#define mulab3rr(XD, XS, XT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulab_rx(W(XD))

#define mulab3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_ld(W(XD), W(MT), W(DT))                                       \
        movab_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulab_rx(W(XD))

#define mulab_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x11))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x11))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x12))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x12))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x13))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x13))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x14))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x14))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x15))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x15))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x16))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x16))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x17))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x17))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x19))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x19))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1F))                              \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlab_ri(XG, IS)                                                    \
        shlab3ri(W(XG), W(XG), W(IS))

#define shlab_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlab3ld(W(XG), W(XG), W(MS), W(DS))

#define shlab3ri(XD, XS, IT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

#define shlab3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

#define shlab_xx() /* not portable, do not use outside */                   \
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
        shlbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrab_ri(XG, IS)                                                    \
        shrab3ri(W(XG), W(XG), W(IS))

#define shrab_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrab3ld(W(XG), W(XG), W(MS), W(DS))

#define shrab3ri(XD, XS, IT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

#define shrab3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

#define shrab_xx() /* not portable, do not use outside */                   \
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
        shrbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrac_ri(XG, IS)                                                    \
        shrac3ri(W(XG), W(XG), W(IS))

#define shrac_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrac3ld(W(XG), W(XG), W(MS), W(DS))

#define shrac3ri(XD, XS, IT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrac_xx()                                                          \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

#define shrac3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrac_xx()                                                          \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

#define shrac_xx() /* not portable, do not use outside */                   \
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
        shrbn_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x11))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x12))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x13))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x15))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x16))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x17))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x19))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1F))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlab_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlab3rr(W(XG), W(XG), W(XS))

#define svlab_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlab3ld(W(XG), W(XG), W(MS), W(DS))

#define svlab3rr(XD, XS, XT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlab_rx(W(XD))

#define svlab3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_ld(W(XD), W(MT), W(DT))                                       \
        movab_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlab_rx(W(XD))

#define svlab_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrab_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrab3rr(W(XG), W(XG), W(XS))

#define svrab_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrab3ld(W(XG), W(XG), W(MS), W(DS))

#define svrab3rr(XD, XS, XT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrab_rx(W(XD))

#define svrab3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_ld(W(XD), W(MT), W(DT))                                       \
        movab_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrab_rx(W(XD))

#define svrab_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrac_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrac3rr(W(XG), W(XG), W(XS))

#define svrac_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrac3ld(W(XG), W(XG), W(MS), W(DS))

#define svrac3rr(XD, XS, XT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrac_rx(W(XD))

#define svrac3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_ld(W(XD), W(MT), W(DT))                                       \
        movab_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrac_rx(W(XD))

#define svrac_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1F))                                    \
        stack_ld(Recx)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minab_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xDA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xDA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minab_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xDA)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xDA)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define minab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        minab_rr(W(XD), W(XT))

#define minab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        minab_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxab_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xDE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xDE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxab_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xDE)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xDE)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define maxab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        maxab_rr(W(XD), W(XT))

#define maxab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        maxab_ld(W(XD), W(MT), W(DT))

#if (RT_SIMD_COMPAT_SSE < 4)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minac_rr(XG, XS)                                                    \
        minac3rr(W(XG), W(XG), W(XS))

#define minac_ld(XG, MS, DS)                                                \
        minac3ld(W(XG), W(XG), W(MS), W(DS))

#define minac3rr(XD, XS, XT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minac_rx(W(XD))

#define minac3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_ld(W(XD), W(MT), W(DT))                                       \
        movab_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minac_rx(W(XD))

#define minac_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x01))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x01))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x01))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x03))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x03))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x03))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x05))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x05))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x05))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x07))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x07))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x07))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x09))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x09))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x09))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0B))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0B))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0B))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0D))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0D))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0D))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0F))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0F))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0F))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x11))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x11))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x11))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x12))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x12))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x12))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x13))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x13))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x13))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x14))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x14))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x14))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x15))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x15))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x15))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x16))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x16))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x16))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x17))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x17))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x17))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x19))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x19))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x19))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1A))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1A))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1A))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1B))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1B))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1B))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1C))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1C))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1C))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1D))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1D))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1D))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1E))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1E))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1E))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1F))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1F))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1F))                              \
        stack_ld(Reax)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR02(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxac_rr(XG, XS)                                                    \
        maxac3rr(W(XG), W(XG), W(XS))

#define maxac_ld(XG, MS, DS)                                                \
        maxac3ld(W(XG), W(XG), W(MS), W(DS))

#define maxac3rr(XD, XS, XT)                                                \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxac_rx(W(XD))

#define maxac3ld(XD, XS, MT, DT)                                            \
        movab_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movab_ld(W(XD), W(MT), W(DT))                                       \
        movab_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxac_rx(W(XD))

#define maxac_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x01))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x01))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x01))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x03))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x03))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x03))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x05))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x05))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x05))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x07))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x07))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x07))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x09))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x09))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x09))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0B))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0B))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0B))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0D))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0D))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0D))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0F))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0F))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0F))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x11))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x11))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x11))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x12))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x12))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x12))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x13))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x13))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x13))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x14))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x14))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x14))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x15))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x15))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x15))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x16))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x16))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x16))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x17))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x17))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x17))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x19))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x19))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x19))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1A))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1A))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1A))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1B))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1B))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1B))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1C))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1C))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1C))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1D))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1D))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1D))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1E))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1E))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1E))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x1F))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x1F))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x1F))                              \
        stack_ld(Reax)                                                      \
        movab_ld(W(XD), Mebp, inf_SCR02(0))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minac_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x38) EMITB(0x38)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x38) EMITB(0x38)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minac_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x38)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x38)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define minac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        minac_rr(W(XD), W(XT))

#define minac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        minac_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxac_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x38) EMITB(0x3C)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x38) EMITB(0x3C)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxac_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3C)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3C)           \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define maxac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        maxac_rr(W(XD), W(XT))

#define maxac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        maxac_ld(W(XD), W(MT), W(DT))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqab_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x74)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x74)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define ceqab_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x74)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x74)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define ceqab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        ceqab_rr(W(XD), W(XT))

#define ceqab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        ceqab_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneab_rr(XG, XS)                                                    \
        ceqab_rr(W(XG), W(XS))                                              \
        notax_rx(W(XG))

#define cneab_ld(XG, MS, DS)                                                \
        ceqab_ld(W(XG), W(MS), W(DS))                                       \
        notax_rx(W(XG))

#define cneab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cneab_rr(W(XD), W(XT))

#define cneab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cneab_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltab_rr(XG, XS)                                                    \
        minab_rr(W(XG), W(XS))                                              \
        cneab_rr(W(XG), W(XS))

#define cltab_ld(XG, MS, DS)                                                \
        minab_ld(W(XG), W(MS), W(DS))                                       \
        cneab_ld(W(XG), W(MS), W(DS))

#define cltab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cltab_rr(W(XD), W(XT))

#define cltab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cltab_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltac_rr(XG, XS)                                                    \
        minac_rr(W(XG), W(XS))                                              \
        cneab_rr(W(XG), W(XS))

#define cltac_ld(XG, MS, DS)                                                \
        minac_ld(W(XG), W(MS), W(DS))                                       \
        cneab_ld(W(XG), W(MS), W(DS))

#define cltac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cltac_rr(W(XD), W(XT))

#define cltac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cltac_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleab_rr(XG, XS)                                                    \
        maxab_rr(W(XG), W(XS))                                              \
        ceqab_rr(W(XG), W(XS))

#define cleab_ld(XG, MS, DS)                                                \
        maxab_ld(W(XG), W(MS), W(DS))                                       \
        ceqab_ld(W(XG), W(MS), W(DS))

#define cleab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cleab_rr(W(XD), W(XT))

#define cleab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cleab_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleac_rr(XG, XS)                                                    \
        cgtac_rr(W(XG), W(XS))                                              \
        notax_rx(W(XG))

#define cleac_ld(XG, MS, DS)                                                \
        cgtac_ld(W(XG), W(MS), W(DS))                                       \
        notax_rx(W(XG))

#define cleac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cleac_rr(W(XD), W(XT))

#define cleac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cleac_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtab_rr(XG, XS)                                                    \
        maxab_rr(W(XG), W(XS))                                              \
        cneab_rr(W(XG), W(XS))

#define cgtab_ld(XG, MS, DS)                                                \
        maxab_ld(W(XG), W(MS), W(DS))                                       \
        cneab_ld(W(XG), W(MS), W(DS))

#define cgtab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cgtab_rr(W(XD), W(XT))

#define cgtab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cgtab_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtac_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x64)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x64)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cgtac_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x64)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x64)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define cgtac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cgtac_rr(W(XD), W(XT))

#define cgtac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cgtac_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeab_rr(XG, XS)                                                    \
        minab_rr(W(XG), W(XS))                                              \
        ceqab_rr(W(XG), W(XS))

#define cgeab_ld(XG, MS, DS)                                                \
        minab_ld(W(XG), W(MS), W(DS))                                       \
        ceqab_ld(W(XG), W(MS), W(DS))

#define cgeab3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cgeab_rr(W(XD), W(XT))

#define cgeab3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cgeab_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeac_rr(XG, XS)                                                    \
        minac_rr(W(XG), W(XS))                                              \
        ceqab_rr(W(XG), W(XS))

#define cgeac_ld(XG, MS, DS)                                                \
        minac_ld(W(XG), W(MS), W(DS))                                       \
        ceqab_ld(W(XG), W(MS), W(DS))

#define cgeac3rr(XD, XS, XT)                                                \
        movab_rr(W(XD), W(XS))                                              \
        cgeac_rr(W(XD), W(XT))

#define cgeac3ld(XD, XS, MT, DT)                                            \
        movab_rr(W(XD), W(XS))                                              \
        cgeac_ld(W(XD), W(MT), W(DT))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_128X2V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

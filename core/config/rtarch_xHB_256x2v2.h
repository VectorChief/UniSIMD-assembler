/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_256X2V2_H
#define RT_RTARCH_XHB_256X2V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_256x2v2.h: Implementation of x86_64 half+byte AVX1/2 pairs.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdm*_rx - applies [cmd] to var-len packed SIMD: [r]egister (one operand)
 * cmdm*_rr - applies [cmd] to var-len packed SIMD: [r]egister from [r]egister
 *
 * cmdm*_rm - applies [cmd] to var-len packed SIMD: [r]egister from [m]emory
 * cmdm*_ld - applies [cmd] to var-len packed SIMD: as above (friendly alias)
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

#if (RT_256X2 >= 1 && RT_256X2 <= 2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmx_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movmx_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

#define movmx_st(XS, MD, DD)                                                \
    ADR VEX(0,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR VEX(1,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VXL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvmx_rr(XG, XS)                                                    \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_rr(Xmm0, W(XG))                                               \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmx_rr(W(XG), Xmm0)

#define mmvmx_ld(XG, MS, DS)                                                \
        notmx_rx(Xmm0)                                                      \
        andmx_rr(W(XG), Xmm0)                                               \
        annmx_ld(Xmm0, W(MS), W(DS))                                        \
        orrmx_rr(W(XG), Xmm0)

#define mmvmx_st(XS, MG, DG)                                                \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_ld(Xmm0, W(MG), W(DG))                                        \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andmx_rr(XG, XS)                                                    \
        andmx3rr(W(XG), W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andmx3ld(W(XG), W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annmx_rr(XG, XS)                                                    \
        annmx3rr(W(XG), W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        annmx3ld(W(XG), W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrmx_rr(XG, XS)                                                    \
        orrmx3rr(W(XG), W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornmx_rr(XG, XS)                                                    \
        notmx_rx(W(XG))                                                     \
        orrmx_rr(W(XG), W(XS))

#define ornmx_ld(XG, MS, DS)                                                \
        notmx_rx(W(XG))                                                     \
        orrmx_ld(W(XG), W(MS), W(DS))

#define ornmx3rr(XD, XS, XT)                                                \
        notmx_rr(W(XD), W(XS))                                              \
        orrmx_rr(W(XD), W(XT))

#define ornmx3ld(XD, XS, MT, DT)                                            \
        notmx_rr(W(XD), W(XS))                                              \
        orrmx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xormx_rr(XG, XS)                                                    \
        xormx3rr(W(XG), W(XG), W(XS))

#define xormx_ld(XG, MS, DS)                                                \
        xormx3ld(W(XG), W(XG), W(MS), W(DS))

#define xormx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xormx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notmx_rx(XG)                                                        \
        notmx_rr(W(XG), W(XG))

#define notmx_rr(XD, XS)                                                    \
        annmx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_256X2 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addmx_rx(W(XD))

#define addmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addmx_rx(W(XD))

#define addmx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsmx3rr(W(XG), W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmx_rx(W(XD))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmx_rx(W(XD))

#define adsmx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsmn3rr(W(XG), W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmn_rx(W(XD))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmn_rx(W(XD))

#define adsmn_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        submx_rx(W(XD))

#define submx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        submx_rx(W(XD))

#define submx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsmx3rr(W(XG), W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmx_rx(W(XD))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmx_rx(W(XD))

#define sbsmx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsmn3rr(W(XG), W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmn_rx(W(XD))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmn_rx(W(XD))

#define sbsmn_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulmx_rx(W(XD))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulmx_rx(W(XD))

#define mulmx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shlgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shlgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrgn_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrgn_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrgn_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrgn_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X2 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsmx3rr(W(XG), W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsmn3rr(W(XG), W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsmx3rr(W(XG), W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsmn3rr(W(XG), W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        VEX(0,             0, REG(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        VEX(0,             1, REH(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        VEX(0,             0, REG(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        VEX(0,             1, REH(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        VEX(0,             0, REG(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        VEX(0,             1, REH(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrmn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

#endif /* RT_256X2 >= 2, AVX2 */

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmx3rr(W(XG), W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlmx_rx(W(XD))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlmx_rx(W(XD))

#define svlmx_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmx3rr(W(XG), W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmx_rx(W(XD))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmx_rx(W(XD))

#define svrmx_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmn3rr(W(XG), W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmn_rx(W(XD))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmn_rx(W(XD))

#define svrmn_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x22))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x26))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x32))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x36))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x3E))                                    \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed half-precision integer compare   ******************/

#if (RT_256X2 < 2)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        minmx3rr(W(XG), W(XG), W(XS))

#define minmx_ld(XG, MS, DS)                                                \
        minmx3ld(W(XG), W(XG), W(MS), W(DS))

#define minmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmx_rx(W(XD))

#define minmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmx_rx(W(XD))

#define minmx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        minmn3rr(W(XG), W(XG), W(XS))

#define minmn_ld(XG, MS, DS)                                                \
        minmn3ld(W(XG), W(XG), W(MS), W(DS))

#define minmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmn_rx(W(XD))

#define minmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmn_rx(W(XD))

#define minmn_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        maxmx3rr(W(XG), W(XG), W(XS))

#define maxmx_ld(XG, MS, DS)                                                \
        maxmx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmx_rx(W(XD))

#define maxmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmx_rx(W(XD))

#define maxmx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        maxmn3rr(W(XG), W(XG), W(XS))

#define maxmn_ld(XG, MS, DS)                                                \
        maxmn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmn_rx(W(XD))

#define maxmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmn_rx(W(XD))

#define maxmn_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqmx3rr(W(XG), W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqmx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        ceqmx_rx(W(XD))

#define ceqmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        ceqmx_rx(W(XD))

#define ceqmx_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtmn3rr(W(XG), W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        cgtmn_rx(W(XD))

#define cgtmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        cgtmn_rx(W(XD))

#define cgtmn_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X2 >= 2, AVX2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        minmx3rr(W(XG), W(XG), W(XS))

#define minmx_ld(XG, MS, DS)                                                \
        minmx3ld(W(XG), W(XG), W(MS), W(DS))

#define minmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        minmn3rr(W(XG), W(XG), W(XS))

#define minmn_ld(XG, MS, DS)                                                \
        minmn3ld(W(XG), W(XG), W(MS), W(DS))

#define minmn3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        maxmx3rr(W(XG), W(XG), W(XS))

#define maxmx_ld(XG, MS, DS)                                                \
        maxmx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        maxmn3rr(W(XG), W(XG), W(XS))

#define maxmn_ld(XG, MS, DS)                                                \
        maxmn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmn3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqmx3rr(W(XG), W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqmx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqmx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtmn3rr(W(XG), W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtmn3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_256X2 >= 2, AVX2 */

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemx_rr(XG, XS)                                                    \
        cnemx3rr(W(XG), W(XG), W(XS))

#define cnemx_ld(XG, MS, DS)                                                \
        cnemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemx3rr(XD, XS, XT)                                                \
        ceqmx3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define cnemx3ld(XD, XS, MT, DT)                                            \
        ceqmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmx_rr(XG, XS)                                                    \
        cltmx3rr(W(XG), W(XG), W(XS))

#define cltmx_ld(XG, MS, DS)                                                \
        cltmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmx3rr(XD, XS, XT)                                                \
        minmx3rr(W(XD), W(XS), W(XT))                                       \
        cnemx_rr(W(XD), W(XT))

#define cltmx3ld(XD, XS, MT, DT)                                            \
        minmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmn_rr(XG, XS)                                                    \
        cltmn3rr(W(XG), W(XG), W(XS))

#define cltmn_ld(XG, MS, DS)                                                \
        cltmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmn3rr(XD, XS, XT)                                                \
        cgtmn3rr(W(XD), W(XT), W(XS))

#define cltmn3ld(XD, XS, MT, DT)                                            \
        minmn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemx_rr(XG, XS)                                                    \
        clemx3rr(W(XG), W(XG), W(XS))

#define clemx_ld(XG, MS, DS)                                                \
        clemx3ld(W(XG), W(XG), W(MS), W(DS))

#define clemx3rr(XD, XS, XT)                                                \
        maxmx3rr(W(XD), W(XS), W(XT))                                       \
        ceqmx_rr(W(XD), W(XT))

#define clemx3ld(XD, XS, MT, DT)                                            \
        maxmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemn_rr(XG, XS)                                                    \
        clemn3rr(W(XG), W(XG), W(XS))

#define clemn_ld(XG, MS, DS)                                                \
        clemn3ld(W(XG), W(XG), W(MS), W(DS))

#define clemn3rr(XD, XS, XT)                                                \
        cgtmn3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define clemn3ld(XD, XS, MT, DT)                                            \
        cgtmn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmx_rr(XG, XS)                                                    \
        cgtmx3rr(W(XG), W(XG), W(XS))

#define cgtmx_ld(XG, MS, DS)                                                \
        cgtmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmx3rr(XD, XS, XT)                                                \
        maxmx3rr(W(XD), W(XS), W(XT))                                       \
        cnemx_rr(W(XD), W(XT))

#define cgtmx3ld(XD, XS, MT, DT)                                            \
        maxmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemx_rr(XG, XS)                                                    \
        cgemx3rr(W(XG), W(XG), W(XS))

#define cgemx_ld(XG, MS, DS)                                                \
        cgemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemx3rr(XD, XS, XT)                                                \
        minmx3rr(W(XD), W(XS), W(XT))                                       \
        ceqmx_rr(W(XD), W(XT))

#define cgemx3ld(XD, XS, MT, DT)                                            \
        minmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemn_rr(XG, XS)                                                    \
        cgemn3rr(W(XG), W(XG), W(XS))

#define cgemn_ld(XG, MS, DS)                                                \
        cgemn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemn3rr(XD, XS, XT)                                                \
        minmn3rr(W(XD), W(XS), W(XT))                                       \
        ceqmx_rr(W(XD), W(XT))

#define cgemn3ld(XD, XS, MT, DT)                                            \
        minmn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmx_ld(W(XD), W(MT), W(DT))

/****************   packed byte-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmb_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movmb_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

#define movmb_st(XS, MD, DD)                                                \
    ADR VEX(0,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR VEX(1,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VXL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvmb_rr(XG, XS)                                                    \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_rr(Xmm0, W(XG))                                               \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmb_rr(W(XG), Xmm0)

#define mmvmb_ld(XG, MS, DS)                                                \
        notmx_rx(Xmm0)                                                      \
        andmx_rr(W(XG), Xmm0)                                               \
        annmx_ld(Xmm0, W(MS), W(DS))                                        \
        orrmx_rr(W(XG), Xmm0)

#define mmvmb_st(XS, MG, DG)                                                \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_ld(Xmm0, W(MG), W(DG))                                        \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmb_st(Xmm0, W(MG), W(DG))

/* logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

#if (RT_256X2 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmb_rr(XG, XS)                                                    \
        addmb3rr(W(XG), W(XG), W(XS))

#define addmb_ld(XG, MS, DS)                                                \
        addmb3ld(W(XG), W(XG), W(MS), W(DS))

#define addmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addmb_rx(W(XD))

#define addmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addmb_rx(W(XD))

#define addmb_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addgb_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addgb_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmb_rr(XG, XS)                                                    \
        adsmb3rr(W(XG), W(XG), W(XS))

#define adsmb_ld(XG, MS, DS)                                                \
        adsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmb_rx(W(XD))

#define adsmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmb_rx(W(XD))

#define adsmb_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsgb_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        adsgb_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmc_rr(XG, XS)                                                    \
        adsmc3rr(W(XG), W(XG), W(XS))

#define adsmc_ld(XG, MS, DS)                                                \
        adsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmc3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmc_rx(W(XD))

#define adsmc3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmc_rx(W(XD))

#define adsmc_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsgc_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        adsgc_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submb_rr(XG, XS)                                                    \
        submb3rr(W(XG), W(XG), W(XS))

#define submb_ld(XG, MS, DS)                                                \
        submb3ld(W(XG), W(XG), W(MS), W(DS))

#define submb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        submb_rx(W(XD))

#define submb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        submb_rx(W(XD))

#define submb_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subgb_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        subgb_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmb_rr(XG, XS)                                                    \
        sbsmb3rr(W(XG), W(XG), W(XS))

#define sbsmb_ld(XG, MS, DS)                                                \
        sbsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmb_rx(W(XD))

#define sbsmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmb_rx(W(XD))

#define sbsmb_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsgb_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        sbsgb_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmc_rr(XG, XS)                                                    \
        sbsmc3rr(W(XG), W(XG), W(XS))

#define sbsmc_ld(XG, MS, DS)                                                \
        sbsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmc3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmc_rx(W(XD))

#define sbsmc3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmc_rx(W(XD))

#define sbsmc_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsgc_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        sbsgc_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X2 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmb_rr(XG, XS)                                                    \
        addmb3rr(W(XG), W(XG), W(XS))

#define addmb_ld(XG, MS, DS)                                                \
        addmb3ld(W(XG), W(XG), W(MS), W(DS))

#define addmb3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addmb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmb_rr(XG, XS)                                                    \
        adsmb3rr(W(XG), W(XG), W(XS))

#define adsmb_ld(XG, MS, DS)                                                \
        adsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmb3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmc_rr(XG, XS)                                                    \
        adsmc3rr(W(XG), W(XG), W(XS))

#define adsmc_ld(XG, MS, DS)                                                \
        adsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmc3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submb_rr(XG, XS)                                                    \
        submb3rr(W(XG), W(XG), W(XS))

#define submb_ld(XG, MS, DS)                                                \
        submb3ld(W(XG), W(XG), W(MS), W(DS))

#define submb3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define submb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmb_rr(XG, XS)                                                    \
        sbsmb3rr(W(XG), W(XG), W(XS))

#define sbsmb_ld(XG, MS, DS)                                                \
        sbsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmb3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmc_rr(XG, XS)                                                    \
        sbsmc3rr(W(XG), W(XG), W(XS))

#define sbsmc_ld(XG, MS, DS)                                                \
        sbsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmc3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_256X2 >= 2, AVX2 */

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmb_rr(XG, XS)                                                    \
        mulmb3rr(W(XG), W(XG), W(XS))

#define mulmb_ld(XG, MS, DS)                                                \
        mulmb3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulmb_rx(W(XD))

#define mulmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulmb_rx(W(XD))

#define mulmb_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR01(0x20))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x20))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x21))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x21))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x22))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x22))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x23))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x23))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x24))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x24))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x25))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x25))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x26))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x26))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x27))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x27))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x28))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x28))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x29))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x29))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2F))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x30))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x30))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x31))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x31))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x32))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x32))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x33))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x33))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x34))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x34))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x35))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x35))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x36))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x36))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x37))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x37))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x38))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x38))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x39))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x39))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3F))                              \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmb_ri(XG, IS)                                                    \
        shlmb3ri(W(XG), W(XG), W(IS))

#define shlmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmb3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmb3ri(XD, XS, IT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#define shlmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#define shlmb_xx() /* not portable, do not use outside */                   \
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
        shlbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmb_ri(XG, IS)                                                    \
        shrmb3ri(W(XG), W(XG), W(IS))

#define shrmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmb3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmb3ri(XD, XS, IT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmb_xx() /* not portable, do not use outside */                   \
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
        shrbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmc_ri(XG, IS)                                                    \
        shrmc3ri(W(XG), W(XG), W(IS))

#define shrmc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmc3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmc3ri(XD, XS, IT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrmc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmc3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrmc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmc_xx() /* not portable, do not use outside */                   \
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
        shrbn_mx(Mebp,  inf_SCR01(0x1F))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x20))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x21))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x22))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x23))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x24))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x25))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x26))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x27))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x28))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x29))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2F))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x30))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x31))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x32))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x33))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x34))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x35))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x36))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x37))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x38))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x39))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3F))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmb3rr(W(XG), W(XG), W(XS))

#define svlmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmb3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlmb_rx(W(XD))

#define svlmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlmb_rx(W(XD))

#define svlmb_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3F))                                    \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmb3rr(W(XG), W(XG), W(XS))

#define svrmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmb3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmb_rx(W(XD))

#define svrmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmb_rx(W(XD))

#define svrmb_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3F))                                    \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmc3rr(W(XG), W(XG), W(XS))

#define svrmc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmc3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmc3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmc_rx(W(XD))

#define svrmc3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmc_rx(W(XD))

#define svrmc_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x21))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x22))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x23))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x25))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x26))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x27))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x29))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x31))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x32))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x33))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x35))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x36))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x37))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x39))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3F))                                    \
        stack_ld(Recx)                                                      \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

#if (RT_256X2 < 2)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmb_rr(XG, XS)                                                    \
        minmb3rr(W(XG), W(XG), W(XS))

#define minmb_ld(XG, MS, DS)                                                \
        minmb3ld(W(XG), W(XG), W(MS), W(DS))

#define minmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmb_rx(W(XD))

#define minmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmb_rx(W(XD))

#define minmb_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mingb_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mingb_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmc_rr(XG, XS)                                                    \
        minmc3rr(W(XG), W(XG), W(XS))

#define minmc_ld(XG, MS, DS)                                                \
        minmc3ld(W(XG), W(XG), W(MS), W(DS))

#define minmc3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmc_rx(W(XD))

#define minmc3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmc_rx(W(XD))

#define minmc_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mingc_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mingc_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmb_rr(XG, XS)                                                    \
        maxmb3rr(W(XG), W(XG), W(XS))

#define maxmb_ld(XG, MS, DS)                                                \
        maxmb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmb_rx(W(XD))

#define maxmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmb_rx(W(XD))

#define maxmb_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxgb_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxgb_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmc_rr(XG, XS)                                                    \
        maxmc3rr(W(XG), W(XG), W(XS))

#define maxmc_ld(XG, MS, DS)                                                \
        maxmc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmc3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmc_rx(W(XD))

#define maxmc3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmc_rx(W(XD))

#define maxmc_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxgc_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxgc_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmb_rr(XG, XS)                                                    \
        ceqmb3rr(W(XG), W(XG), W(XS))

#define ceqmb_ld(XG, MS, DS)                                                \
        ceqmb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmb3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        ceqmb_rx(W(XD))

#define ceqmb3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        ceqmb_rx(W(XD))

#define ceqmb_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        ceqgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        ceqgb_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        ceqgb_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmc_rr(XG, XS)                                                    \
        cgtmc3rr(W(XG), W(XG), W(XS))

#define cgtmc_ld(XG, MS, DS)                                                \
        cgtmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmc3rr(XD, XS, XT)                                                \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        cgtmc_rx(W(XD))

#define cgtmc3ld(XD, XS, MT, DT)                                            \
        movmb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmb_ld(W(XD), W(MT), W(DT))                                       \
        movmb_st(W(XD), Mebp, inf_SCR02(0))                                 \
        cgtmc_rx(W(XD))

#define cgtmc_rx(XD) /* not portable, do not use outside */                 \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        cgtgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        cgtgc_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movgb_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        cgtgc_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movgb_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movmb_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X2 >= 2, AVX2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmb_rr(XG, XS)                                                    \
        minmb3rr(W(XG), W(XG), W(XS))

#define minmb_ld(XG, MS, DS)                                                \
        minmb3ld(W(XG), W(XG), W(MS), W(DS))

#define minmb3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmc_rr(XG, XS)                                                    \
        minmc3rr(W(XG), W(XG), W(XS))

#define minmc_ld(XG, MS, DS)                                                \
        minmc3ld(W(XG), W(XG), W(MS), W(DS))

#define minmc3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmb_rr(XG, XS)                                                    \
        maxmb3rr(W(XG), W(XG), W(XS))

#define maxmb_ld(XG, MS, DS)                                                \
        maxmb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmb3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmc_rr(XG, XS)                                                    \
        maxmc3rr(W(XG), W(XG), W(XS))

#define maxmc_ld(XG, MS, DS)                                                \
        maxmc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmc3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmb_rr(XG, XS)                                                    \
        ceqmb3rr(W(XG), W(XG), W(XS))

#define ceqmb_ld(XG, MS, DS)                                                \
        ceqmb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmb3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqmb3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmc_rr(XG, XS)                                                    \
        cgtmc3rr(W(XG), W(XG), W(XS))

#define cgtmc_ld(XG, MS, DS)                                                \
        cgtmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmc3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtmc3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_256X2 >= 2, AVX2 */

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemb_rr(XG, XS)                                                    \
        cnemb3rr(W(XG), W(XG), W(XS))

#define cnemb_ld(XG, MS, DS)                                                \
        cnemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemb3rr(XD, XS, XT)                                                \
        ceqmb3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define cnemb3ld(XD, XS, MT, DT)                                            \
        ceqmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmb_rr(XG, XS)                                                    \
        cltmb3rr(W(XG), W(XG), W(XS))

#define cltmb_ld(XG, MS, DS)                                                \
        cltmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmb3rr(XD, XS, XT)                                                \
        minmb3rr(W(XD), W(XS), W(XT))                                       \
        cnemb_rr(W(XD), W(XT))

#define cltmb3ld(XD, XS, MT, DT)                                            \
        minmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemb_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmc_rr(XG, XS)                                                    \
        cltmc3rr(W(XG), W(XG), W(XS))

#define cltmc_ld(XG, MS, DS)                                                \
        cltmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmc3rr(XD, XS, XT)                                                \
        cgtmc3rr(W(XD), W(XT), W(XS))

#define cltmc3ld(XD, XS, MT, DT)                                            \
        minmc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemb_rr(XG, XS)                                                    \
        clemb3rr(W(XG), W(XG), W(XS))

#define clemb_ld(XG, MS, DS)                                                \
        clemb3ld(W(XG), W(XG), W(MS), W(DS))

#define clemb3rr(XD, XS, XT)                                                \
        maxmb3rr(W(XD), W(XS), W(XT))                                       \
        ceqmb_rr(W(XD), W(XT))

#define clemb3ld(XD, XS, MT, DT)                                            \
        maxmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemc_rr(XG, XS)                                                    \
        clemc3rr(W(XG), W(XG), W(XS))

#define clemc_ld(XG, MS, DS)                                                \
        clemc3ld(W(XG), W(XG), W(MS), W(DS))

#define clemc3rr(XD, XS, XT)                                                \
        cgtmc3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define clemc3ld(XD, XS, MT, DT)                                            \
        cgtmc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmb_rr(XG, XS)                                                    \
        cgtmb3rr(W(XG), W(XG), W(XS))

#define cgtmb_ld(XG, MS, DS)                                                \
        cgtmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmb3rr(XD, XS, XT)                                                \
        maxmb3rr(W(XD), W(XS), W(XT))                                       \
        cnemb_rr(W(XD), W(XT))

#define cgtmb3ld(XD, XS, MT, DT)                                            \
        maxmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemb_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemb_rr(XG, XS)                                                    \
        cgemb3rr(W(XG), W(XG), W(XS))

#define cgemb_ld(XG, MS, DS)                                                \
        cgemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemb3rr(XD, XS, XT)                                                \
        minmb3rr(W(XD), W(XS), W(XT))                                       \
        ceqmb_rr(W(XD), W(XT))

#define cgemb3ld(XD, XS, MT, DT)                                            \
        minmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmb_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemc_rr(XG, XS)                                                    \
        cgemc3rr(W(XG), W(XG), W(XS))

#define cgemc_ld(XG, MS, DS)                                                \
        cgemc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemc3rr(XD, XS, XT)                                                \
        minmc3rr(W(XD), W(XS), W(XT))                                       \
        ceqmb_rr(W(XD), W(XT))

#define cgemc3ld(XD, XS, MT, DT)                                            \
        minmc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmb_ld(W(XD), W(MT), W(DT))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_256X2V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

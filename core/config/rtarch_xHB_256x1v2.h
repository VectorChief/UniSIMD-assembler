/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_256X1V2_H
#define RT_RTARCH_XHB_256X1V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_256x1v2.h: Implementation of x86_64 half+byte AVX1/2 instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmda*_rx - applies [cmd] to 256-bit packed-half: [r]egister (one operand)
 * cmda*_rr - applies [cmd] to 256-bit packed-half: [r]egister from [r]egister
 *
 * cmda*_rm - applies [cmd] to 256-bit packed-half: [r]egister from [m]emory
 * cmda*_ld - applies [cmd] to 256-bit packed-half: as above (friendly alias)
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

#if (RT_256X1 >= 1 && RT_256X1 <= 2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movax_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movax_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movax_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

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
        andax3rr(W(XG), W(XG), W(XS))

#define andax_ld(XG, MS, DS)                                                \
        andax3ld(W(XG), W(XG), W(MS), W(DS))

#define andax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annax_rr(XG, XS)                                                    \
        annax3rr(W(XG), W(XG), W(XS))

#define annax_ld(XG, MS, DS)                                                \
        annax3ld(W(XG), W(XG), W(MS), W(DS))

#define annax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrax_rr(XG, XS)                                                    \
        orrax3rr(W(XG), W(XG), W(XS))

#define orrax_ld(XG, MS, DS)                                                \
        orrax3ld(W(XG), W(XG), W(MS), W(DS))

#define orrax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

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
        xorax3rr(W(XG), W(XG), W(XS))

#define xorax_ld(XG, MS, DS)                                                \
        xorax3ld(W(XG), W(XG), W(MS), W(DS))

#define xorax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notax_rx(XG)                                                        \
        notax_rr(W(XG), W(XG))

#define notax_rr(XD, XS)                                                    \
        annax3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_256X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addax_rx(W(XD))

#define addax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addax_rx(W(XD))

#define addax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
        adsax3rr(W(XG), W(XG), W(XS))

#define adsax_ld(XG, MS, DS)                                                \
        adsax3ld(W(XG), W(XG), W(MS), W(DS))

#define adsax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsax_rx(W(XD))

#define adsax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsax_rx(W(XD))

#define adsax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
        adsan3rr(W(XG), W(XG), W(XS))

#define adsan_ld(XG, MS, DS)                                                \
        adsan3ld(W(XG), W(XG), W(MS), W(DS))

#define adsan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsan_rx(W(XD))

#define adsan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsan_rx(W(XD))

#define adsan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        subax_rx(W(XD))

#define subax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        subax_rx(W(XD))

#define subax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
        sbsax3rr(W(XG), W(XG), W(XS))

#define sbsax_ld(XG, MS, DS)                                                \
        sbsax3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsax_rx(W(XD))

#define sbsax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsax_rx(W(XD))

#define sbsax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
        sbsan3rr(W(XG), W(XG), W(XS))

#define sbsan_ld(XG, MS, DS)                                                \
        sbsan3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsan_rx(W(XD))

#define sbsan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsan_rx(W(XD))

#define sbsan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulax_rx(W(XD))

#define mulax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulax_rx(W(XD))

#define mulax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shlax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shrax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shran3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
        adsax3rr(W(XG), W(XG), W(XS))

#define adsax_ld(XG, MS, DS)                                                \
        adsax3ld(W(XG), W(XG), W(MS), W(DS))

#define adsax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
        adsan3rr(W(XG), W(XG), W(XS))

#define adsan_ld(XG, MS, DS)                                                \
        adsan3ld(W(XG), W(XG), W(MS), W(DS))

#define adsan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
        sbsax3rr(W(XG), W(XG), W(XS))

#define sbsax_ld(XG, MS, DS)                                                \
        sbsax3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
        sbsan3rr(W(XG), W(XG), W(XS))

#define sbsan_ld(XG, MS, DS)                                                \
        sbsan3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shran3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

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

#if (RT_256X1 < 2)

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
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minan_rr(XG, XS)                                                    \
        minan3rr(W(XG), W(XG), W(XS))

#define minan_ld(XG, MS, DS)                                                \
        minan3ld(W(XG), W(XG), W(MS), W(DS))

#define minan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minan_rx(W(XD))

#define minan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minan_rx(W(XD))

#define minan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

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
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxan_rr(XG, XS)                                                    \
        maxan3rr(W(XG), W(XG), W(XS))

#define maxan_ld(XG, MS, DS)                                                \
        maxan3ld(W(XG), W(XG), W(MS), W(DS))

#define maxan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxan_rx(W(XD))

#define maxan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxan_rx(W(XD))

#define maxan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqax_rr(XG, XS)                                                    \
        ceqax3rr(W(XG), W(XG), W(XS))

#define ceqax_ld(XG, MS, DS)                                                \
        ceqax3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        ceqax_rx(W(XD))

#define ceqax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        ceqax_rx(W(XD))

#define ceqax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtan_rr(XG, XS)                                                    \
        cgtan3rr(W(XG), W(XG), W(XS))

#define cgtan_ld(XG, MS, DS)                                                \
        cgtan3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        cgtan_rx(W(XD))

#define cgtan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        cgtan_rx(W(XD))

#define cgtan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minax_rr(XG, XS)                                                    \
        minax3rr(W(XG), W(XG), W(XS))

#define minax_ld(XG, MS, DS)                                                \
        minax3ld(W(XG), W(XG), W(MS), W(DS))

#define minax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minan_rr(XG, XS)                                                    \
        minan3rr(W(XG), W(XG), W(XS))

#define minan_ld(XG, MS, DS)                                                \
        minan3ld(W(XG), W(XG), W(MS), W(DS))

#define minan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxax_rr(XG, XS)                                                    \
        maxax3rr(W(XG), W(XG), W(XS))

#define maxax_ld(XG, MS, DS)                                                \
        maxax3ld(W(XG), W(XG), W(MS), W(DS))

#define maxax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxan_rr(XG, XS)                                                    \
        maxan3rr(W(XG), W(XG), W(XS))

#define maxan_ld(XG, MS, DS)                                                \
        maxan3ld(W(XG), W(XG), W(MS), W(DS))

#define maxan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqax_rr(XG, XS)                                                    \
        ceqax3rr(W(XG), W(XG), W(XS))

#define ceqax_ld(XG, MS, DS)                                                \
        ceqax3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtan_rr(XG, XS)                                                    \
        cgtan3rr(W(XG), W(XG), W(XS))

#define cgtan_ld(XG, MS, DS)                                                \
        cgtan3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneax_rr(XG, XS)                                                    \
        cneax3rr(W(XG), W(XG), W(XS))

#define cneax_ld(XG, MS, DS)                                                \
        cneax3ld(W(XG), W(XG), W(MS), W(DS))

#define cneax3rr(XD, XS, XT)                                                \
        ceqax3rr(W(XD), W(XS), W(XT))                                       \
        notax_rx(W(XD))

#define cneax3ld(XD, XS, MT, DT)                                            \
        ceqax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notax_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltax_rr(XG, XS)                                                    \
        cltax3rr(W(XG), W(XG), W(XS))

#define cltax_ld(XG, MS, DS)                                                \
        cltax3ld(W(XG), W(XG), W(MS), W(DS))

#define cltax3rr(XD, XS, XT)                                                \
        minax3rr(W(XD), W(XS), W(XT))                                       \
        cneax_rr(W(XD), W(XT))

#define cltax3ld(XD, XS, MT, DT)                                            \
        minax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneax_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltan_rr(XG, XS)                                                    \
        cltan3rr(W(XG), W(XG), W(XS))

#define cltan_ld(XG, MS, DS)                                                \
        cltan3ld(W(XG), W(XG), W(MS), W(DS))

#define cltan3rr(XD, XS, XT)                                                \
        cgtan3rr(W(XD), W(XT), W(XS))

#define cltan3ld(XD, XS, MT, DT)                                            \
        minan3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneax_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleax_rr(XG, XS)                                                    \
        cleax3rr(W(XG), W(XG), W(XS))

#define cleax_ld(XG, MS, DS)                                                \
        cleax3ld(W(XG), W(XG), W(MS), W(DS))

#define cleax3rr(XD, XS, XT)                                                \
        maxax3rr(W(XD), W(XS), W(XT))                                       \
        ceqax_rr(W(XD), W(XT))

#define cleax3ld(XD, XS, MT, DT)                                            \
        maxax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqax_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clean_rr(XG, XS)                                                    \
        clean3rr(W(XG), W(XG), W(XS))

#define clean_ld(XG, MS, DS)                                                \
        clean3ld(W(XG), W(XG), W(MS), W(DS))

#define clean3rr(XD, XS, XT)                                                \
        cgtan3rr(W(XD), W(XS), W(XT))                                       \
        notax_rx(W(XD))

#define clean3ld(XD, XS, MT, DT)                                            \
        cgtan3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notax_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtax_rr(XG, XS)                                                    \
        cgtax3rr(W(XG), W(XG), W(XS))

#define cgtax_ld(XG, MS, DS)                                                \
        cgtax3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtax3rr(XD, XS, XT)                                                \
        maxax3rr(W(XD), W(XS), W(XT))                                       \
        cneax_rr(W(XD), W(XT))

#define cgtax3ld(XD, XS, MT, DT)                                            \
        maxax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneax_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeax_rr(XG, XS)                                                    \
        cgeax3rr(W(XG), W(XG), W(XS))

#define cgeax_ld(XG, MS, DS)                                                \
        cgeax3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeax3rr(XD, XS, XT)                                                \
        minax3rr(W(XD), W(XS), W(XT))                                       \
        ceqax_rr(W(XD), W(XT))

#define cgeax3ld(XD, XS, MT, DT)                                            \
        minax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqax_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgean_rr(XG, XS)                                                    \
        cgean3rr(W(XG), W(XG), W(XS))

#define cgean_ld(XG, MS, DS)                                                \
        cgean3ld(W(XG), W(XG), W(MS), W(DS))

#define cgean3rr(XD, XS, XT)                                                \
        minan3rr(W(XD), W(XS), W(XT))                                       \
        ceqax_rr(W(XD), W(XT))

#define cgean3ld(XD, XS, MT, DT)                                            \
        minan3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqax_ld(W(XD), W(MT), W(DT))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_256X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

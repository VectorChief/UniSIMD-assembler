/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_512X2V2_H
#define RT_RTARCH_XHB_512X2V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_512x2v2.h: Implementation of x86_64 half+byte AVX512F/BW pairs.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdm*_ri - applies [cmd] to [p]acked: [r]egister from [i]mmediate
 * cmdm*_rr - applies [cmd] to [p]acked: [r]egister from [r]egister
 *
 * cmdm*_rm - applies [cmd] to [p]acked: [r]egister from [m]emory
 * cmdm*_ld - applies [cmd] to [p]acked: as above
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

#if (RT_512X2 >= 1 && RT_512X2 <= 2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmx_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movmx_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#define movmx_st(XS, MD, DD)                                                \
    ADR EVX(RXB(XS), RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR EVX(RMB(XS), RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VZL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#if (RT_512X2 == 1)

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

#else /* RT_512X2 == 2 */

#define mmvmx_rr(XG, XS)                                                    \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(RXB(XG), RXB(XS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1mx_rm(XmmG, Mebp, inf_GPC07)                                     \
        EKW(RMB(XG), RMB(XS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvmx_ld(XG, MS, DS)                                                \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XG), RXB(MS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        ck1mx_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKW(RMB(XG), RXB(MS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#define mmvmx_st(XS, MG, DG)                                                \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XS), RXB(MG),    0x00, K, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VAL(DG)), EMPTY)                                 \
        ck1mx_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKW(RMB(XS), RXB(MG),    0x00, K, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VZL(DG)), EMPTY)

#define ck1mx_rm(XS, MT, DT) /* not portable, do not use outside */         \
    ADR EVX(0,       RXB(MT), REN(XS), K, 1, 1) EMITB(0x75)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

#endif /* RT_512X2 == 2 */

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andmx_rr(XG, XS)                                                    \
        andmx3rr(W(XG), W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andmx3ld(W(XG), W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annmx_rr(XG, XS)                                                    \
        annmx3rr(W(XG), W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        annmx3ld(W(XG), W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrmx_rr(XG, XS)                                                    \
        orrmx3rr(W(XG), W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xormx_rr(XG, XS)                                                    \
        xormx3rr(W(XG), W(XG), W(XS))

#define xormx_ld(XG, MS, DS)                                                \
        xormx3ld(W(XG), W(XG), W(MS), W(DS))

#define xormx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xormx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notmx_rx(XG)                                                        \
        notmx_rr(W(XG), W(XG))

#define notmx_rr(XD, XS)                                                    \
        annmx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_512X2 < 2)

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define addmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define addmx_xx() /* not portable, do not use outside */                   \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        submx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define submx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        submx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define submx_xx() /* not portable, do not use outside */                   \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define mulmx_xx() /* not portable, do not use outside */                   \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlax3ri(W(XD), W(XS), W(IT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrax3ri(W(XD), W(XS), W(IT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shran3ri(W(XD), W(XS), W(IT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shran3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmx3rr(W(XG), W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svlmx_xx() /* not portable, do not use outside */                   \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x40))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x40))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x42))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x42))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x44))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x44))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x46))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x46))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x48))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x48))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x4A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x4C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x4E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x50))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x50))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x52))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x52))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x54))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x54))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x56))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x56))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x58))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x58))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x5A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x5C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x5E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x60))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x60))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x62))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x62))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x64))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x64))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x66))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x66))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x68))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x68))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x6A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x6C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x6E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x70))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x70))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x72))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x72))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x74))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x74))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x76))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x76))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x78))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x78))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x7A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x7C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x7E))                                    \
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmx3rr(W(XG), W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmx_xx() /* not portable, do not use outside */                   \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x40))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x40))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x42))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x42))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x44))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x44))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x46))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x46))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x48))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x48))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x4A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x4C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x4E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x50))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x50))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x52))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x52))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x54))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x54))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x56))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x56))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x58))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x58))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x5A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x5C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x5E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x60))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x60))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x62))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x62))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x64))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x64))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x66))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x66))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x68))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x68))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x6A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x6C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x6E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x70))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x70))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x72))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x72))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x74))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x74))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x76))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x76))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x78))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x78))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x7A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x7C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x7E))                                    \
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmn3rr(W(XG), W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmn_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmn_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmn_xx() /* not portable, do not use outside */                   \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x40))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x40))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x42))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x42))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x44))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x44))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x46))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x46))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x48))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x48))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x4A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x4C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x4E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x4E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x50))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x50))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x52))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x52))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x54))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x54))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x56))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x56))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x58))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x58))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x5A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x5C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x5E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x5E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x60))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x60))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x62))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x62))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x64))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x64))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x66))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x66))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x68))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x68))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x6A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x6C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x6E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x6E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x70))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x70))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x72))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x72))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x74))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x74))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x76))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x76))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x78))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x78))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x7A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x7C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x7E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x7E))                                    \
        stack_ld(Recx)

#else /* RT_512X2 >= 2 */

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x0F))                        \
        EVX(0,       RMB(XS), REM(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x0F))

#define shlmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x0F))                        \
        EVX(0,       RMB(XS), REM(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x0F))

#define shrmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x0F))                        \
        EVX(0,       RMB(XS), REM(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x0F))

#define shrmn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmx3rr(W(XG), W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmx3rr(W(XG), W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmn3rr(W(XG), W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

#endif /* RT_512X2 >= 2 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_512X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_512X2V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

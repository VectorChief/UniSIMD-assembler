/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_512X4V2_H
#define RT_RTARCH_XHB_512X4V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_512x4v2.h: Implementation of x86_64 half+byte AVX512F/BW quads.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdm*_rx - applies [cmd] to var-len packed-half: [r]egister (one operand)
 * cmdm*_rr - applies [cmd] to var-len packed-half: [r]egister from [r]egister
 *
 * cmdm*_rm - applies [cmd] to var-len packed-half: [r]egister from [m]emory
 * cmdm*_ld - applies [cmd] to var-len packed-half: as above (friendly alias)
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

#if (RT_512X4 >= 1 && RT_512X4 <= 2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmx_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movmx_ld(XD, MS, DS)                                                \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

#define movmx_st(XS, MD, DD)                                                \
    ADR EVX(0,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR EVX(1,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VZL(DD)), EMPTY)                                 \
    ADR EVX(2,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VSL(DD)), EMPTY)                                 \
    ADR EVX(3,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VTL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#if (RT_512X4 == 1)

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

#else /* RT_512X4 == 2 */

#define mmvmx_rr(XG, XS)                                                    \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(0,             0,    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1mx_rm(Xmm8, Mebp, inf_GPC07)                                     \
        EKW(1,             1,    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1mx_rm(XmmG, Mebp, inf_GPC07)                                     \
        EKW(2,             2,    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1mx_rm(XmmO, Mebp, inf_GPC07)                                     \
        EKW(3,             3,    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvmx_ld(XG, MS, DS)                                                \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(0,       RXB(MS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        ck1mx_rm(Xmm8, Mebp, inf_GPC07)                                     \
    ADR EKW(1,       RXB(MS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
        ck1mx_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKW(2,       RXB(MS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
        ck1mx_rm(XmmO, Mebp, inf_GPC07)                                     \
    ADR EKW(3,       RXB(MS),    0x00, K, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

#define mmvmx_st(XS, MG, DG)                                                \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(0,       RXB(MG),    0x00, K, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VAL(DG)), EMPTY)                                 \
        ck1mx_rm(Xmm8, Mebp, inf_GPC07)                                     \
    ADR EKW(1,       RXB(MG),    0x00, K, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VZL(DG)), EMPTY)                                 \
        ck1mx_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKW(2,       RXB(MG),    0x00, K, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VSL(DG)), EMPTY)                                 \
        ck1mx_rm(XmmO, Mebp, inf_GPC07)                                     \
    ADR EKW(3,       RXB(MG),    0x00, K, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VTL(DG)), EMPTY)

#define ck1mx_rm(XS, MT, DT) /* not portable, do not use outside */         \
    ADR EVX(0,       RXB(MT), REN(XS), K, 1, 1) EMITB(0x75)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

#endif /* RT_512X4 == 2 */

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andmx_rr(XG, XS)                                                    \
        andmx3rr(W(XG), W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andmx3ld(W(XG), W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annmx_rr(XG, XS)                                                    \
        annmx3rr(W(XG), W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        annmx3ld(W(XG), W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrmx_rr(XG, XS)                                                    \
        orrmx3rr(W(XG), W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

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
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xormx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notmx_rx(XG)                                                        \
        notmx_rr(W(XG), W(XG))

#define notmx_rr(XD, XS)                                                    \
        annmx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_512X4 < 2)

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
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x80))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x80))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x82))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x82))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x84))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x84))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x86))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x86))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x88))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x88))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x8A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x8C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x8E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x90))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x90))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x92))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x92))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x94))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x94))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x96))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x96))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x98))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x98))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x9A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x9C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x9E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA0))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xA0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA2))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xA2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA4))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xA4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA6))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xA6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA8))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xA8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAA))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xAA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAC))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xAC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAE))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xAE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB0))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xB0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB2))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xB2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB4))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xB4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB6))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xB6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB8))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xB8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBA))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xBA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBC))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xBC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBE))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xBE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC0))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xC0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC2))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xC2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC4))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xC4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC6))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xC6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC8))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xC8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCA))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xCA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCC))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xCC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCE))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xCE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD0))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xD0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD2))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xD2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD4))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xD4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD6))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xD6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD8))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xD8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDA))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xDA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDC))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xDC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDE))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xDE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE0))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xE0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE2))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xE2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE4))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xE4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE6))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xE6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE8))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xE8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEA))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xEA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEC))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xEC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEE))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xEE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF0))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xF0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF2))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xF2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF4))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xF4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF6))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xF6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF8))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xF8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFA))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xFA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFC))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xFC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFE))                              \
        shlhx_mx(Mebp,  inf_SCR01(0xFE))                                    \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x80))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x80))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x82))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x82))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x84))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x84))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x86))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x86))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x88))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x88))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x8A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x8C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x8E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x90))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x90))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x92))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x92))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x94))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x94))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x96))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x96))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x98))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x98))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x9A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x9C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x9E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA0))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xA0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA2))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xA2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA4))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xA4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA6))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xA6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA8))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xA8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAA))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xAA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAC))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xAC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAE))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xAE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB0))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xB0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB2))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xB2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB4))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xB4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB6))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xB6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB8))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xB8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBA))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xBA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBC))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xBC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBE))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xBE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC0))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xC0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC2))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xC2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC4))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xC4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC6))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xC6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC8))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xC8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCA))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xCA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCC))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xCC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCE))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xCE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD0))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xD0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD2))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xD2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD4))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xD4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD6))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xD6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD8))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xD8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDA))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xDA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDC))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xDC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDE))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xDE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE0))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xE0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE2))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xE2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE4))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xE4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE6))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xE6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE8))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xE8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEA))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xEA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEC))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xEC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEE))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xEE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF0))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xF0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF2))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xF2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF4))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xF4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF6))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xF6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF8))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xF8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFA))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xFA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFC))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xFC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFE))                              \
        shrhx_mx(Mebp,  inf_SCR01(0xFE))                                    \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x80))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x80))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x82))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x82))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x84))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x84))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x86))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x86))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x88))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x88))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x8A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x8C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x8E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x8E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x90))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x90))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x92))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x92))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x94))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x94))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x96))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x96))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x98))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x98))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x9A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x9C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x9E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x9E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA0))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xA0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA2))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xA2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA4))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xA4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA6))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xA6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xA8))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xA8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAA))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xAA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAC))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xAC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xAE))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xAE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB0))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xB0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB2))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xB2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB4))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xB4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB6))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xB6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xB8))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xB8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBA))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xBA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBC))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xBC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xBE))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xBE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC0))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xC0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC2))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xC2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC4))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xC4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC6))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xC6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xC8))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xC8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCA))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xCA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCC))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xCC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xCE))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xCE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD0))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xD0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD2))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xD2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD4))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xD4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD6))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xD6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xD8))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xD8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDA))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xDA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDC))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xDC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xDE))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xDE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE0))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xE0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE2))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xE2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE4))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xE4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE6))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xE6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xE8))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xE8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEA))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xEA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEC))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xEC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xEE))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xEE))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF0))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xF0))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF2))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xF2))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF4))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xF4))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF6))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xF6))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xF8))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xF8))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFA))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xFA))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFC))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xFC))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0xFE))                              \
        shrhn_mx(Mebp,  inf_SCR01(0xFE))                                    \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_512X4 >= 2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsmx3rr(W(XG), W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsmn3rr(W(XG), W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsmx3rr(W(XG), W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsmn3rr(W(XG), W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        EVX(0,             0, REG(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             1, REH(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             2, REI(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             3, REJ(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        EVX(0,             0, REG(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             1, REH(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             2, REI(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             3, REJ(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        EVX(0,             0, REG(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             1, REH(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             2, REI(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             3, REJ(XD), K, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrmn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmx3rr(W(XG), W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(1,             1, REH(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(2,             2, REI(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(3,             3, REJ(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVW(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVW(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVW(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmx3rr(W(XG), W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(1,             1, REH(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(2,             2, REI(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(3,             3, REJ(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVW(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVW(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVW(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmn3rr(W(XG), W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(1,             1, REH(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(2,             2, REI(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVW(3,             3, REJ(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVW(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVW(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVW(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

#endif /* RT_512X4 >= 2 */

/*****************   packed half-precision integer compare   ******************/

#if (RT_512X4 < 2)

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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
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
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x40))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0x40))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x40))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x60))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0x60))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x60))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x80))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0x80))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x80))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xA0))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0xA0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xA0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xC0))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0xC0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xC0))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0xE0))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0xE0))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0xE0))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

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

#else /* RT_512X4 >= 2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        minmx3rr(W(XG), W(XG), W(XS))

#define minmx_ld(XG, MS, DS)                                                \
        minmx3ld(W(XG), W(XG), W(MS), W(DS))

#define minmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        minmn3rr(W(XG), W(XG), W(XS))

#define minmn_ld(XG, MS, DS)                                                \
        minmn3ld(W(XG), W(XG), W(MS), W(DS))

#define minmn3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        maxmx3rr(W(XG), W(XG), W(XS))

#define maxmx_ld(XG, MS, DS)                                                \
        maxmx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmx3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        maxmn3rr(W(XG), W(XG), W(XS))

#define maxmn_ld(XG, MS, DS)                                                \
        maxmn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmn3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqmx3rr(W(XG), W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqmx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define ceqmx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x00))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x00))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x00))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x00))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemx_rr(XG, XS)                                                    \
        cnemx3rr(W(XG), W(XG), W(XS))

#define cnemx_ld(XG, MS, DS)                                                \
        cnemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cnemx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x04))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x04))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x04))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x04))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmx_rr(XG, XS)                                                    \
        cltmx3rr(W(XG), W(XG), W(XS))

#define cltmx_ld(XG, MS, DS)                                                \
        cltmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cltmx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x01))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x01))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x01))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x01))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmn_rr(XG, XS)                                                    \
        cltmn3rr(W(XG), W(XG), W(XS))

#define cltmn_ld(XG, MS, DS)                                                \
        cltmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmn3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cltmn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x01))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x01))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x01))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x01))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemx_rr(XG, XS)                                                    \
        clemx3rr(W(XG), W(XG), W(XS))

#define clemx_ld(XG, MS, DS)                                                \
        clemx3ld(W(XG), W(XG), W(MS), W(DS))

#define clemx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define clemx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x02))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x02))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x02))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x02))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemn_rr(XG, XS)                                                    \
        clemn3rr(W(XG), W(XG), W(XS))

#define clemn_ld(XG, MS, DS)                                                \
        clemn3ld(W(XG), W(XG), W(MS), W(DS))

#define clemn3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define clemn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x02))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x02))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x02))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x02))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmx_rr(XG, XS)                                                    \
        cgtmx3rr(W(XG), W(XG), W(XS))

#define cgtmx_ld(XG, MS, DS)                                                \
        cgtmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cgtmx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x06))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x06))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x06))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x06))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtmn3rr(W(XG), W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cgtmn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x06))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x06))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x06))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x06))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemx_rr(XG, XS)                                                    \
        cgemx3rr(W(XG), W(XG), W(XS))

#define cgemx_ld(XG, MS, DS)                                                \
        cgemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemx3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cgemx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x05))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x05))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x05))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x05))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemn_rr(XG, XS)                                                    \
        cgemn3rr(W(XG), W(XG), W(XS))

#define cgemn_ld(XG, MS, DS)                                                \
        cgemn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemn3rr(XD, XS, XT)                                                \
        EVW(0,             0, REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             1, REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             2, REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVW(0,             3, REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cgemn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x05))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x05))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x05))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x05))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)


#define mz1mx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR EZW(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x66)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_512X4 >= 2 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_512X4 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_512X4V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

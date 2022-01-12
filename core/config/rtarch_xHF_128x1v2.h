/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHF_128X1V2_H
#define RT_RTARCH_XHF_128X1V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHF_128x1v2.h: Implementation of x86_64 AVX512VL/fp16 instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * and contains architecture-specific extensions
 * outside of the common assembler core.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdg*_rx - applies [cmd] to 128-bit packed-fp16: [r]egister (one operand)
 * cmdg*_rr - applies [cmd] to 128-bit packed-fp16: [r]egister from [r]egister
 *
 * cmdg*_rm - applies [cmd] to 128-bit packed-fp16: [r]egister from [m]emory
 * cmdg*_ld - applies [cmd] to 128-bit packed-fp16: as above (friendly alias)
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

#if (RT_128X1 == 2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmgx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movns_st(W(XS), W(MD), W(DD))

/*************   packed half-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define neggs_rx(XG)                                                        \
        neggs_rr(W(XG), W(XG))

#define neggs_rr(XD, XS)                                                    \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), Mebp, inf_GPC06_32)                                 \
        shrix_ri(W(XD), IB(16))                                             \
        xorix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        xorix_ld(W(XD), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgs_rr(XG, XS)                                                    \
        addgs3rr(W(XG), W(XG), W(XS))

#define addgs_ld(XG, MS, DS)                                                \
        addgs3ld(W(XG), W(XG), W(MS), W(DS))

#define addgs3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addgs3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgs_rr(XG, XS)                                                    \
        subgs3rr(W(XG), W(XG), W(XS))

#define subgs_ld(XG, MS, DS)                                                \
        subgs3ld(W(XG), W(XG), W(MS), W(DS))

#define subgs3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subgs3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgs_rr(XG, XS)                                                    \
        mulgs3rr(W(XG), W(XG), W(XS))

#define mulgs_ld(XG, MS, DS)                                                \
        mulgs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgs3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulgs3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divgs_rr(XG, XS)                                                    \
        divgs3rr(W(XG), W(XG), W(XS))

#define divgs_ld(XG, MS, DS)                                                \
        divgs3ld(W(XG), W(XG), W(MS), W(DS))

#define divgs3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divgs3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqrgs_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrgs_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 0, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcegs_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsgs_rr(XG, XS) /* destroys XS */                                  \
        mulgs_rr(W(XS), W(XG))                                              \
        mulgs_rr(W(XS), W(XG))                                              \
        addgs_rr(W(XG), W(XG))                                              \
        subgs_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsegs_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmags_rr(XG, XS, XT)                                                \
    ADR EFX(RXB(XG), RXB(XT), REN(XS), 0, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmags_ld(XG, XS, MT, DT)                                            \
    ADR EFX(RXB(XG), RXB(MT), REN(XS), 0, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsgs_rr(XG, XS, XT)                                                \
    ADR EFX(RXB(XG), RXB(XT), REN(XS), 0, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsgs_ld(XG, XS, MT, DT)                                            \
    ADR EFX(RXB(XG), RXB(MT), REN(XS), 0, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mings_rr(XG, XS)                                                    \
        mings3rr(W(XG), W(XG), W(XS))

#define mings_ld(XG, MS, DS)                                                \
        mings3ld(W(XG), W(XG), W(MS), W(DS))

#define mings3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mings3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxgs_rr(XG, XS)                                                    \
        maxgs3rr(W(XG), W(XG), W(XS))

#define maxgs_ld(XG, MS, DS)                                                \
        maxgs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgs3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgs3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgs_rr(XG, XS)                                                    \
        ceqgs3rr(W(XG), W(XG), W(XS))

#define ceqgs_ld(XG, MS, DS)                                                \
        ceqgs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define ceqgs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegs_rr(XG, XS)                                                    \
        cnegs3rr(W(XG), W(XG), W(XS))

#define cnegs_ld(XG, MS, DS)                                                \
        cnegs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cnegs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltgs_rr(XG, XS)                                                    \
        cltgs3rr(W(XG), W(XG), W(XS))

#define cltgs_ld(XG, MS, DS)                                                \
        cltgs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cltgs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clegs_rr(XG, XS)                                                    \
        clegs3rr(W(XG), W(XG), W(XS))

#define clegs_ld(XG, MS, DS)                                                \
        clegs3ld(W(XG), W(XG), W(MS), W(DS))

#define clegs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define clegs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtgs_rr(XG, XS)                                                    \
        cgtgs3rr(W(XG), W(XG), W(XS))

#define cgtgs_ld(XG, MS, DS)                                                \
        cgtgs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cgtgs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgegs_rr(XG, XS)                                                    \
        cgegs3rr(W(XG), W(XG), W(XS))

#define cgegs_ld(XG, MS, DS)                                                \
        cgegs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cgegs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

    /* mkj for half-precision is defined in corresponding HB_128 header */

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzgs_rr(XD, XS)     /* round towards zero */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzgs_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzgs_rr(XD, XS)     /* round towards zero */                       \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzgs_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 0, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpgs_rr(XD, XS)     /* round towards +inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpgs_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpgs_rr(XD, XS)     /* round towards +inf */                       \
        EGX(RXB(XD), RXB(XS),    0x00, 2, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpgs_ld(XD, MS, DS) /* round towards +inf */                       \
        movgx_ld(W(XD), W(MS), W(DS))                                       \
        cvpgs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmgs_rr(XD, XS)     /* round towards -inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmgs_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmgs_rr(XD, XS)     /* round towards -inf */                       \
        EGX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmgs_ld(XD, MS, DS) /* round towards -inf */                       \
        movgx_ld(W(XD), W(MS), W(DS))                                       \
        cvmgs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnngs_rr(XD, XS)     /* round towards near */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnngs_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvngs_rr(XD, XS)     /* round towards near */                       \
        cvtgs_rr(W(XD), W(XS))

#define cvngs_ld(XD, MS, DS) /* round towards near */                       \
        cvtgs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvngn_rr(XD, XS)     /* round towards near */                       \
        cvtgn_rr(W(XD), W(XS))

#define cvngn_ld(XD, MS, DS) /* round towards near */                       \
        cvtgn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndgs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndgs_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtgs_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtgs_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 0, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtgn_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtgn_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 0, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrgs_rr(XD, XS, mode)                                              \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrgs_rr(XD, XS, mode)                                              \
        EGX(RXB(XD), RXB(XS), 0x00, RT_SIMD_MODE_##mode&3, 2, 1) EMITB(0x7D)\
        MRM(REG(XD), MOD(XS), REG(XS))

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/**********   scalar half-precision floating-point move/arithmetic   **********/

/* mov (D = S) */

#define movns_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS), REN(XD), 0, 2, 1) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movns_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 0, 2, 1) EMITB(0x10)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movns_st(XS, MD, DD)                                                \
    ADR EFX(RXB(XS), RXB(MD),    0x00, 0, 2, 1) EMITB(0x11)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addns_rr(XG, XS)                                                    \
        addns3rr(W(XG), W(XG), W(XS))

#define addns_ld(XG, MS, DS)                                                \
        addns3ld(W(XG), W(XG), W(MS), W(DS))

#define addns3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addns3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subns_rr(XG, XS)                                                    \
        subns3rr(W(XG), W(XG), W(XS))

#define subns_ld(XG, MS, DS)                                                \
        subns3ld(W(XG), W(XG), W(MS), W(DS))

#define subns3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subns3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulns_rr(XG, XS)                                                    \
        mulns3rr(W(XG), W(XG), W(XS))

#define mulns_ld(XG, MS, DS)                                                \
        mulns3ld(W(XG), W(XG), W(MS), W(DS))

#define mulns3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulns3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divns_rr(XG, XS)                                                    \
        divns3rr(W(XG), W(XG), W(XS))

#define divns_ld(XG, MS, DS)                                                \
        divns3ld(W(XG), W(XG), W(MS), W(DS))

#define divns3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divns3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqrns_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 2, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrns_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 0, 2, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcens_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 1, 2) EMITB(0x4D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsns_rr(XG, XS) /* destroys XS */                                  \
        mulns_rr(W(XS), W(XG))                                              \
        mulns_rr(W(XS), W(XG))                                              \
        addns_rr(W(XG), W(XG))                                              \
        subns_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsens_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 0, 1, 2) EMITB(0x4F)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmans_rr(XG, XS, XT)                                                \
    ADR EFX(RXB(XG), RXB(XT), REN(XS), 0, 1, 2) EMITB(0xB9)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmans_ld(XG, XS, MT, DT)                                            \
    ADR EFX(RXB(XG), RXB(MT), REN(XS), 0, 1, 2) EMITB(0xB9)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsns_rr(XG, XS, XT)                                                \
    ADR EFX(RXB(XG), RXB(XT), REN(XS), 0, 1, 2) EMITB(0xBD)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsns_ld(XG, XS, MT, DT)                                            \
    ADR EFX(RXB(XG), RXB(MT), REN(XS), 0, 1, 2) EMITB(0xBD)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/**************   scalar half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minns_rr(XG, XS)                                                    \
        minns3rr(W(XG), W(XG), W(XS))

#define minns_ld(XG, MS, DS)                                                \
        minns3ld(W(XG), W(XG), W(MS), W(DS))

#define minns3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minns3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxns_rr(XG, XS)                                                    \
        maxns3rr(W(XG), W(XG), W(XS))

#define maxns_ld(XG, MS, DS)                                                \
        maxns3ld(W(XG), W(XG), W(MS), W(DS))

#define maxns3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 0, 2, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxns3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 0, 2, 1) EMITB(0x5F)                 \
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

#endif /* RT_RTARCH_XHF_128X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

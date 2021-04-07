/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHF_128X1V2_H
#define RT_RTARCH_AHF_128X1V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHF_128x1v2.h: Implementation of AArch64 fp16 NEON instructions.
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

#if (RT_128X1 != 0)

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

#if (RT_128X1 == 2 || RT_128X1 == 8)

/* neg (G = -G), (D = -S) */

#define neggs_rx(XG)                                                        \
        neggs_rr(W(XG), W(XG))

#define neggs_rr(XD, XS)                                                    \
        EMITW(0x6EF8F800 | MXM(REG(XD), REG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgs_rr(XG, XS)                                                    \
        addgs3rr(W(XG), W(XG), W(XS))

#define addgs_ld(XG, MS, DS)                                                \
        addgs3ld(W(XG), W(XG), W(MS), W(DS))

#define addgs3rr(XD, XS, XT)                                                \
        EMITW(0x4E401400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E401400 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgs_rr(XG, XS)                                                    \
        subgs3rr(W(XG), W(XG), W(XS))

#define subgs_ld(XG, MS, DS)                                                \
        subgs3ld(W(XG), W(XG), W(MS), W(DS))

#define subgs3rr(XD, XS, XT)                                                \
        EMITW(0x4EC01400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EC01400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgs_rr(XG, XS)                                                    \
        mulgs3rr(W(XG), W(XG), W(XS))

#define mulgs_ld(XG, MS, DS)                                                \
        mulgs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgs3rr(XD, XS, XT)                                                \
        EMITW(0x6E401C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E401C00 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divgs_rr(XG, XS)                                                    \
        divgs3rr(W(XG), W(XG), W(XS))

#define divgs_ld(XG, MS, DS)                                                \
        divgs3ld(W(XG), W(XG), W(MS), W(DS))

#define divgs3rr(XD, XS, XT)                                                \
        EMITW(0x6E403C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define divgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E403C00 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrgs_rr(XD, XS)                                                    \
        EMITW(0x6EF9F800 | MXM(REG(XD), REG(XS), 0x00))

#define sqrgs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EF9F800 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S) */

#define rcegs_rr(XD, XS)                                                    \
        EMITW(0x4EF9D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsgs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E403C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E401C00 | MXM(REG(XG), REG(XG), REG(XS)))

/* rsq (D = 1.0 / sqrt S) */

#define rsegs_rr(XD, XS)                                                    \
        EMITW(0x6EF9D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssgs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E401C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EC03C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E401C00 | MXM(REG(XG), REG(XG), REG(XS)))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmags_rr(XG, XS, XT)                                                \
        EMITW(0x4E400C00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmags_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E400C00 | MXM(REG(XG), REG(XS), TmmM))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsgs_rr(XG, XS, XT)                                                \
        EMITW(0x4EC00C00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsgs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EC00C00 | MXM(REG(XG), REG(XS), TmmM))

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mings_rr(XG, XS)                                                    \
        mings3rr(W(XG), W(XG), W(XS))

#define mings_ld(XG, MS, DS)                                                \
        mings3ld(W(XG), W(XG), W(MS), W(DS))

#define mings3rr(XD, XS, XT)                                                \
        EMITW(0x4EC03400 | MXM(REG(XD), REG(XS), REG(XT)))

#define mings3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EC03400 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxgs_rr(XG, XS)                                                    \
        maxgs3rr(W(XG), W(XG), W(XS))

#define maxgs_ld(XG, MS, DS)                                                \
        maxgs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgs3rr(XD, XS, XT)                                                \
        EMITW(0x4E403400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E403400 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgs_rr(XG, XS)                                                    \
        ceqgs3rr(W(XG), W(XG), W(XS))

#define ceqgs_ld(XG, MS, DS)                                                \
        ceqgs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgs3rr(XD, XS, XT)                                                \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegs_rr(XG, XS)                                                    \
        cnegs3rr(W(XG), W(XG), W(XS))

#define cnegs_ld(XG, MS, DS)                                                \
        cnegs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegs3rr(XD, XS, XT)                                                \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cnegs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltgs_rr(XG, XS)                                                    \
        cltgs3rr(W(XG), W(XG), W(XS))

#define cltgs_ld(XG, MS, DS)                                                \
        cltgs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgs3rr(XD, XS, XT)                                                \
        EMITW(0x6EC02400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EC02400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clegs_rr(XG, XS)                                                    \
        clegs3rr(W(XG), W(XG), W(XS))

#define clegs_ld(XG, MS, DS)                                                \
        clegs3ld(W(XG), W(XG), W(MS), W(DS))

#define clegs3rr(XD, XS, XT)                                                \
        EMITW(0x6E402400 | MXM(REG(XD), REG(XT), REG(XS)))

#define clegs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E402400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtgs_rr(XG, XS)                                                    \
        cgtgs3rr(W(XG), W(XG), W(XS))

#define cgtgs_ld(XG, MS, DS)                                                \
        cgtgs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgs3rr(XD, XS, XT)                                                \
        EMITW(0x6EC02400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtgs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EC02400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgegs_rr(XG, XS)                                                    \
        cgegs3rr(W(XG), W(XG), W(XS))

#define cgegs_ld(XG, MS, DS)                                                \
        cgegs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegs3rr(XD, XS, XT)                                                \
        EMITW(0x6E402400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgegs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E402400 | MXM(REG(XD), REG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_128     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_128     0x08    /*  all satisfy the condition */

#define mkjgx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4E71B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x0E022C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwxZri(Reax, IB(RT_SIMD_MASK_##mask##16_128))                     \
        jezxx_lb(lb)

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzgs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EF99800 | MXM(REG(XD), REG(XS), 0x00))

#define rnzgs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EF99800 | MXM(REG(XD), TmmM,    0x00))

#define cvzgs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EF9B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvzgs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EF9B800 | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpgs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EF98800 | MXM(REG(XD), REG(XS), 0x00))

#define rnpgs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EF98800 | MXM(REG(XD), TmmM,    0x00))

#define cvpgs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EF9A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvpgs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EF9A800 | MXM(REG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmgs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E799800 | MXM(REG(XD), REG(XS), 0x00))

#define rnmgs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E799800 | MXM(REG(XD), TmmM,    0x00))

#define cvmgs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E79B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvmgs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E79B800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnngs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E798800 | MXM(REG(XD), REG(XS), 0x00))

#define rnngs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E798800 | MXM(REG(XD), TmmM,    0x00))

#define cvngs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E79A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvngs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E79A800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvngn_rr(XD, XS)     /* round towards near */                       \
        cvtgn_rr(W(XD), W(XS))

#define cvngn_ld(XD, MS, DS) /* round towards near */                       \
        cvtgn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndgs_rr(XD, XS)                                                    \
        EMITW(0x6EF99800 | MXM(REG(XD), REG(XS), 0x00))

#define rndgs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EF99800 | MXM(REG(XD), TmmM,    0x00))

#define cvtgs_rr(XD, XS)                                                    \
        rndgs_rr(W(XD), W(XS))                                              \
        cvzgs_rr(W(XD), W(XD))

#define cvtgs_ld(XD, MS, DS)                                                \
        rndgs_ld(W(XD), W(MS), W(DS))                                       \
        cvzgs_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtgn_rr(XD, XS)                                                    \
        EMITW(0x4E79D800 | MXM(REG(XD), REG(XS), 0x00))

#define cvtgn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E79D800 | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrgs_rr(XD, XS, mode)                                              \
        EMITW(0x4E798800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvrgs_rr(XD, XS, mode)                                              \
        EMITW(0x4E79A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#endif /* RT_128X1 == 2, 8 */

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/**********   scalar half-precision floating-point move/arithmetic   **********/

/* mov (D = S) */

#define movns_rr(XD, XS)                                                    \
        EMITW(0x5E020400 | MXM(REG(XD), REG(XS), 0x00))

#define movns_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x7C400000 | MPM(REG(XD), MOD(MS), VHL(DS), B1(DS), P1(DS)))

#define movns_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x7C000000 | MPM(REG(XS), MOD(MD), VHL(DD), B1(DD), P1(DD)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addns_rr(XG, XS)                                                    \
        addns3rr(W(XG), W(XG), W(XS))

#define addns_ld(XG, MS, DS)                                                \
        addns3ld(W(XG), W(XG), W(MS), W(DS))

#define addns3rr(XD, XS, XT)                                                \
        EMITW(0x1EE02800 | MXM(REG(XD), REG(XS), REG(XT)))

#define addns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1EE02800 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subns_rr(XG, XS)                                                    \
        subns3rr(W(XG), W(XG), W(XS))

#define subns_ld(XG, MS, DS)                                                \
        subns3ld(W(XG), W(XG), W(MS), W(DS))

#define subns3rr(XD, XS, XT)                                                \
        EMITW(0x1EE03800 | MXM(REG(XD), REG(XS), REG(XT)))

#define subns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1EE03800 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulns_rr(XG, XS)                                                    \
        mulns3rr(W(XG), W(XG), W(XS))

#define mulns_ld(XG, MS, DS)                                                \
        mulns3ld(W(XG), W(XG), W(MS), W(DS))

#define mulns3rr(XD, XS, XT)                                                \
        EMITW(0x1EE00800 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1EE00800 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divns_rr(XG, XS)                                                    \
        divns3rr(W(XG), W(XG), W(XS))

#define divns_ld(XG, MS, DS)                                                \
        divns3ld(W(XG), W(XG), W(MS), W(DS))

#define divns3rr(XD, XS, XT)                                                \
        EMITW(0x1EE01800 | MXM(REG(XD), REG(XS), REG(XT)))

#define divns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1EE01800 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrns_rr(XD, XS)                                                    \
        EMITW(0x1EE1C000 | MXM(REG(XD), REG(XS), 0x00))

#define sqrns_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1EE1C000 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S) */

#define rcens_rr(XD, XS)                                                    \
        EMITW(0x5EF9D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsns_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x5E403C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1EE00800 | MXM(REG(XG), REG(XG), REG(XS)))

/* rsq (D = 1.0 / sqrt S) */

#define rsens_rr(XD, XS)                                                    \
        EMITW(0x7EF9D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssns_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1EE00800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x5EC03C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1EE00800 | MXM(REG(XG), REG(XG), REG(XS)))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmans_rr(XG, XS, XT)                                                \
        EMITW(0x1FC00000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmans_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1FC00000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsns_rr(XG, XS, XT)                                                \
        EMITW(0x1FC08000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmsns_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1FC08000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

/**************   scalar half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minns_rr(XG, XS)                                                    \
        minns3rr(W(XG), W(XG), W(XS))

#define minns_ld(XG, MS, DS)                                                \
        minns3ld(W(XG), W(XG), W(MS), W(DS))

#define minns3rr(XD, XS, XT)                                                \
        EMITW(0x1EE05800 | MXM(REG(XD), REG(XS), REG(XT)))

#define minns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1EE05800 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxns_rr(XG, XS)                                                    \
        maxns3rr(W(XG), W(XG), W(XS))

#define maxns_ld(XG, MS, DS)                                                \
        maxns3ld(W(XG), W(XG), W(MS), W(DS))

#define maxns3rr(XD, XS, XT)                                                \
        EMITW(0x1EE04800 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1EE04800 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqns_rr(XG, XS)                                                    \
        ceqns3rr(W(XG), W(XG), W(XS))

#define ceqns_ld(XG, MS, DS)                                                \
        ceqns3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqns3rr(XD, XS, XT)                                                \
        EMITW(0x5E402400 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x5E402400 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnens_rr(XG, XS)                                                    \
        cnens3rr(W(XG), W(XG), W(XS))

#define cnens_ld(XG, MS, DS)                                                \
        cnens3ld(W(XG), W(XG), W(MS), W(DS))

#define cnens3rr(XD, XS, XT)                                                \
        EMITW(0x5E402400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cnens3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x5E402400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltns_rr(XG, XS)                                                    \
        cltns3rr(W(XG), W(XG), W(XS))

#define cltns_ld(XG, MS, DS)                                                \
        cltns3ld(W(XG), W(XG), W(MS), W(DS))

#define cltns3rr(XD, XS, XT)                                                \
        EMITW(0x7EC02400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7EC02400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clens_rr(XG, XS)                                                    \
        clens3rr(W(XG), W(XG), W(XS))

#define clens_ld(XG, MS, DS)                                                \
        clens3ld(W(XG), W(XG), W(MS), W(DS))

#define clens3rr(XD, XS, XT)                                                \
        EMITW(0x7E402400 | MXM(REG(XD), REG(XT), REG(XS)))

#define clens3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7E402400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtns_rr(XG, XS)                                                    \
        cgtns3rr(W(XG), W(XG), W(XS))

#define cgtns_ld(XG, MS, DS)                                                \
        cgtns3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtns3rr(XD, XS, XT)                                                \
        EMITW(0x7EC02400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtns3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7EC02400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgens_rr(XG, XS)                                                    \
        cgens3rr(W(XG), W(XG), W(XS))

#define cgens_ld(XG, MS, DS)                                                \
        cgens3ld(W(XG), W(XG), W(MS), W(DS))

#define cgens3rr(XD, XS, XT)                                                \
        EMITW(0x7E402400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgens3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7E402400 | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_AHF_128X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

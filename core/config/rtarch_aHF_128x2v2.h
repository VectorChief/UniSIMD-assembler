/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHF_128X2V2_H
#define RT_RTARCH_AHF_128X2V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHF_128x2v2.h: Implementation of AArch64 fp16 NEON instruction pairs.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * and contains architecture-specific extensions
 * outside of the common assembler core.
 *
 * Recommended naming scheme for instructions:
 *
 * cmda*_rx - applies [cmd] to 256-bit packed-fp16: [r]egister (one operand)
 * cmda*_rr - applies [cmd] to 256-bit packed-fp16: [r]egister from [r]egister
 *
 * cmda*_rm - applies [cmd] to 256-bit packed-fp16: [r]egister from [m]emory
 * cmda*_ld - applies [cmd] to 256-bit packed-fp16: as above (friendly alias)
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

#if (RT_128X2 != 0) && (RT_SIMD_COMPAT_XMM > 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmax_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmgx_st(W(XS), W(MD), W(DD))

/*************   packed half-precision floating-point arithmetic   ************/

#if (RT_128X2 == 2)

/* neg (G = -G), (D = -S) */

#define negas_rx(XG)                                                        \
        negas_rr(W(XG), W(XG))

#define negas_rr(XD, XS)                                                    \
        EMITW(0x6EF8F800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EF8F800 | MXM(RYG(XD), RYG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addas_rr(XG, XS)                                                    \
        addas3rr(W(XG), W(XG), W(XS))

#define addas_ld(XG, MS, DS)                                                \
        addas3ld(W(XG), W(XG), W(MS), W(DS))

#define addas3rr(XD, XS, XT)                                                \
        EMITW(0x4E401400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E401400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E401400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E401400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subas_rr(XG, XS)                                                    \
        subas3rr(W(XG), W(XG), W(XS))

#define subas_ld(XG, MS, DS)                                                \
        subas3ld(W(XG), W(XG), W(MS), W(DS))

#define subas3rr(XD, XS, XT)                                                \
        EMITW(0x4EC01400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EC01400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EC01400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EC01400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulas_rr(XG, XS)                                                    \
        mulas3rr(W(XG), W(XG), W(XS))

#define mulas_ld(XG, MS, DS)                                                \
        mulas3ld(W(XG), W(XG), W(MS), W(DS))

#define mulas3rr(XD, XS, XT)                                                \
        EMITW(0x6E401C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E401C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E401C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E401C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divas_rr(XG, XS)                                                    \
        divas3rr(W(XG), W(XG), W(XS))

#define divas_ld(XG, MS, DS)                                                \
        divas3ld(W(XG), W(XG), W(MS), W(DS))

#define divas3rr(XD, XS, XT)                                                \
        EMITW(0x6E403C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E403C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E403C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E403C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqras_rr(XD, XS)                                                    \
        EMITW(0x6EF9F800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EF9F800 | MXM(RYG(XD), RYG(XS), 0x00))

#define sqras_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EF9F800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EF9F800 | MXM(RYG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S) */

#define rceas_rr(XD, XS)                                                    \
        EMITW(0x4EF9D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EF9D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rcsas_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E403C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E401C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E403C00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E401C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

/* rsq (D = 1.0 / sqrt S) */

#define rseas_rr(XD, XS)                                                    \
        EMITW(0x6EF9D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EF9D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rssas_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E401C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EC03C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E401C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E401C00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x4EC03C00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E401C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmaas_rr(XG, XS, XT)                                                \
        EMITW(0x4E400C00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4E400C00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmaas_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E400C00 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E400C00 | MXM(RYG(XG), RYG(XS), TmmM))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsas_rr(XG, XS, XT)                                                \
        EMITW(0x4EC00C00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4EC00C00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsas_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EC00C00 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EC00C00 | MXM(RYG(XG), RYG(XS), TmmM))

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minas_rr(XG, XS)                                                    \
        minas3rr(W(XG), W(XG), W(XS))

#define minas_ld(XG, MS, DS)                                                \
        minas3ld(W(XG), W(XG), W(MS), W(DS))

#define minas3rr(XD, XS, XT)                                                \
        EMITW(0x4EC03400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EC03400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EC03400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EC03400 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxas_rr(XG, XS)                                                    \
        maxas3rr(W(XG), W(XG), W(XS))

#define maxas_ld(XG, MS, DS)                                                \
        maxas3ld(W(XG), W(XG), W(MS), W(DS))

#define maxas3rr(XD, XS, XT)                                                \
        EMITW(0x4E403400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E403400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E403400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E403400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqas_rr(XG, XS)                                                    \
        ceqas3rr(W(XG), W(XG), W(XS))

#define ceqas_ld(XG, MS, DS)                                                \
        ceqas3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqas3rr(XD, XS, XT)                                                \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E402400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E402400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneas_rr(XG, XS)                                                    \
        cneas3rr(W(XG), W(XG), W(XS))

#define cneas_ld(XG, MS, DS)                                                \
        cneas3ld(W(XG), W(XG), W(MS), W(DS))

#define cneas3rr(XD, XS, XT)                                                \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x4E402400 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

#define cneas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E402400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E402400 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltas_rr(XG, XS)                                                    \
        cltas3rr(W(XG), W(XG), W(XS))

#define cltas_ld(XG, MS, DS)                                                \
        cltas3ld(W(XG), W(XG), W(MS), W(DS))

#define cltas3rr(XD, XS, XT)                                                \
        EMITW(0x6EC02400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6EC02400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EC02400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EC02400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleas_rr(XG, XS)                                                    \
        cleas3rr(W(XG), W(XG), W(XS))

#define cleas_ld(XG, MS, DS)                                                \
        cleas3ld(W(XG), W(XG), W(MS), W(DS))

#define cleas3rr(XD, XS, XT)                                                \
        EMITW(0x6E402400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6E402400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E402400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E402400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtas_rr(XG, XS)                                                    \
        cgtas3rr(W(XG), W(XG), W(XS))

#define cgtas_ld(XG, MS, DS)                                                \
        cgtas3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtas3rr(XD, XS, XT)                                                \
        EMITW(0x6EC02400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EC02400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EC02400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EC02400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeas_rr(XG, XS)                                                    \
        cgeas3rr(W(XG), W(XG), W(XS))

#define cgeas_ld(XG, MS, DS)                                                \
        cgeas3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeas3rr(XD, XS, XT)                                                \
        EMITW(0x6E402400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E402400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeas3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E402400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E402400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_256     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_256     0x08    /*  all satisfy the condition */

#define mkjax_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4E201C00 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                                (0x08 - RT_SIMD_MASK_##mask##16_256) << 20) \
        EMITW(0x4E71B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x0E022C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwxZri(Reax, IB(RT_SIMD_MASK_##mask##16_256))                     \
        jezxx_lb(lb)

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzas_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EF99800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EF99800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnzas_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF99800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF99800 | MXM(RYG(XD), TmmM,    0x00))

#define cvzas_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EF9B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EF9B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvzas_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF9B800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF9B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpas_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EF98800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EF98800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnpas_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF98800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF98800 | MXM(RYG(XD), TmmM,    0x00))

#define cvpas_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EF9A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EF9A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvpas_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF9A800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EF9A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmas_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E799800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E799800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnmas_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E799800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E799800 | MXM(RYG(XD), TmmM,    0x00))

#define cvmas_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E79B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E79B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvmas_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E79B800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E79B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnnas_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E798800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E798800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnnas_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E798800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E798800 | MXM(RYG(XD), TmmM,    0x00))

#define cvnas_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E79A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E79A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvnas_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E79A800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E79A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnan_rr(XD, XS)     /* round towards near */                       \
        cvtan_rr(W(XD), W(XS))

#define cvnan_ld(XD, MS, DS) /* round towards near */                       \
        cvtan_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndas_rr(XD, XS)                                                    \
        EMITW(0x6EF99800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EF99800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rndas_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EF99800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EF99800 | MXM(RYG(XD), TmmM,    0x00))

#define cvtas_rr(XD, XS)                                                    \
        rndas_rr(W(XD), W(XS))                                              \
        cvzas_rr(W(XD), W(XD))

#define cvtas_ld(XD, MS, DS)                                                \
        rndas_ld(W(XD), W(MS), W(DS))                                       \
        cvzas_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtan_rr(XD, XS)                                                    \
        EMITW(0x4E79D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E79D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtan_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E79D800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E79D800 | MXM(RYG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnras_rr(XD, XS, mode)                                              \
        EMITW(0x4E798800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E798800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvras_rr(XD, XS, mode)                                              \
        EMITW(0x4E79A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E79A800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#endif /* RT_128X2 == 2 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_AHF_128X2V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

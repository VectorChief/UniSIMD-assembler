/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_MHB_128X2V1_H
#define RT_RTARCH_MHB_128X2V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_mHB_128x2v1.h: Implementation of MIPS half+byte MSA pairs.
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

#if (RT_128X2 != 0) && (RT_SIMD_COMPAT_XMM > 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movax_rr(XD, XS)                                                    \
        EMITW(0x78BE0019 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x78BE0019 | MXM(RYG(XD), RYG(XS), 0x00))

#define movax_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(REG(XD), MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x78000023 | MPM(RYG(XD), MOD(MS), VYL(DS), B4(DS), L2(DS)))

#define movax_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMITW(0x78000027 | MPM(REG(XS), MOD(MD), VAL(DD), B4(DD), L2(DD)))  \
        EMITW(0x78000027 | MPM(RYG(XS), MOD(MD), VYL(DD), B4(DD), L2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvax_rr(XG, XS)                                                    \
        EMITW(0x7880001E | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x7880001E | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvax_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x7880001E | MXM(REG(XG), TmmM,    Tmm0))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x7880001E | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvax_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A2(DG), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x7880001E | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x78000027 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))  \
        EMITW(0x7880001E | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
        EMITW(0x78000027 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andax_rr(XG, XS)                                                    \
        andax3rr(W(XG), W(XG), W(XS))

#define andax_ld(XG, MS, DS)                                                \
        andax3ld(W(XG), W(XG), W(MS), W(DS))

#define andax3rr(XD, XS, XT)                                                \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7800001E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7800001E | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annax_rr(XG, XS)                                                    \
        EMITW(0x78C0001E | MXM(REG(XG), REG(XS), TmmZ))                     \
        EMITW(0x78C0001E | MXM(RYG(XG), RYG(XS), TmmZ))

#define annax_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x78C0001E | MXM(REG(XG), TmmM,    TmmZ))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x78C0001E | MXM(RYG(XG), TmmM,    TmmZ))

#define annax3rr(XD, XS, XT)                                                \
        movax_rr(W(XD), W(XS))                                              \
        annax_rr(W(XD), W(XT))

#define annax3ld(XD, XS, MT, DT)                                            \
        movax_rr(W(XD), W(XS))                                              \
        annax_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrax_rr(XG, XS)                                                    \
        orrax3rr(W(XG), W(XG), W(XS))

#define orrax_ld(XG, MS, DS)                                                \
        orrax3ld(W(XG), W(XG), W(MS), W(DS))

#define orrax3rr(XD, XS, XT)                                                \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7820001E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7820001E | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorax_rr(XG, XS)                                                    \
        xorax3rr(W(XG), W(XG), W(XS))

#define xorax_ld(XG, MS, DS)                                                \
        xorax3ld(W(XG), W(XG), W(MS), W(DS))

#define xorax3rr(XD, XS, XT)                                                \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7860001E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xorax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7860001E | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notax_rx(XG)                                                        \
        notax_rr(W(XG), W(XG))

#define notax_rr(XD, XS)                                                    \
        EMITW(0x7840001E | MXM(REG(XD), TmmZ,    REG(XS)))                  \
        EMITW(0x7840001E | MXM(RYG(XD), TmmZ,    RYG(XS)))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        EMITW(0x7820000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7820000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7820000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7820000E | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        EMITW(0x78A0000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78A0000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x78A0000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x78A0000E | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        EMITW(0x78200012 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78200012 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x78200012 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x78200012 | MXM(RYG(XD), RYG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        EMITW(0x78600009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x0F & VAL(IT)) << 16)    \
        EMITW(0x78600009 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
                                                 (0x0F & VAL(IT)) << 16)

#define shlax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x84000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B01001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7820000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7820000D | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        EMITW(0x79600009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x0F & VAL(IT)) << 16)    \
        EMITW(0x79600009 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
                                                 (0x0F & VAL(IT)) << 16)

#define shrax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x84000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B01001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7920000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7920000D | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        EMITW(0x78E00009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x0F & VAL(IT)) << 16)    \
        EMITW(0x78E00009 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
                                                 (0x0F & VAL(IT)) << 16)

#define shran3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x84000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B01001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x78A0000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78A0000D | MXM(RYG(XD), RYG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlax3rr(W(XG), W(XG), W(XS))

#define svlax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlax3ld(W(XG), W(XG), W(MS), W(DS))

#define svlax3rr(XD, XS, XT)                                                \
        EMITW(0x7820000D | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7820000D | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svlax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7820000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7820000D | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrax3rr(W(XG), W(XG), W(XS))

#define svrax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrax3ld(W(XG), W(XG), W(MS), W(DS))

#define svrax3rr(XD, XS, XT)                                                \
        EMITW(0x7920000D | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7920000D | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svrax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7920000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x7920000D | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svran_rr(XG, XS)     /* variable shift with per-elem count */       \
        svran3rr(W(XG), W(XG), W(XS))

#define svran_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svran3ld(W(XG), W(XG), W(MS), W(DS))

#define svran3rr(XD, XS, XT)                                                \
        EMITW(0x78A0000D | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78A0000D | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svran3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x78A0000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x78A0000D | MXM(RYG(XD), RYG(XS), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_MHB_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

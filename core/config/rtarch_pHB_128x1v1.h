/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_PHB_128X1V1_H
#define RT_RTARCH_PHB_128X1V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_pHB_128x1v1.h: Implementation of POWER half+byte VSX1/2 instructions.
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

#if (RT_128X1 == 1 || RT_128X1 == 4)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))

#define movgx_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(REG(XD), Teax & M(MOD(MS) == TPxx), TPxx))

#define movgx_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(REG(XS), Teax & M(MOD(MD) == TPxx), TPxx))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgx_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))

#define mmvgx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))

#define mmvgx_st(XS, MG, DG)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MG) == TPxx), TPxx))   \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    Teax & M(MOD(MG) == TPxx), TPxx))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
        andgx3rr(W(XG), W(XG), W(XS))

#define andgx_ld(XG, MS, DS)                                                \
        andgx3ld(W(XG), W(XG), W(MS), W(DS))

#define andgx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), REG(XT)))

#define andgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anngx_rr(XG, XS)                                                    \
        anngx3rr(W(XG), W(XG), W(XS))

#define anngx_ld(XG, MS, DS)                                                \
        anngx3ld(W(XG), W(XG), W(MS), W(DS))

#define anngx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000457 | MXM(REG(XD), REG(XT), REG(XS)))

#define anngx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0xF0000457 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrgx_rr(XG, XS)                                                    \
        orrgx3rr(W(XG), W(XG), W(XS))

#define orrgx_ld(XG, MS, DS)                                                \
        orrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrgx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#if (RT_SIMD_COMPAT_PW8 == 0)

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

#else /* RT_SIMD_COMPAT_PW8 == 1 */

#define orngx_rr(XG, XS)                                                    \
        orngx3rr(W(XG), W(XG), W(XS))

#define orngx_ld(XG, MS, DS)                                                \
        orngx3ld(W(XG), W(XG), W(MS), W(DS))

#define orngx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000557 | MXM(REG(XD), REG(XT), REG(XS)))

#define orngx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0xF0000557 | MXM(REG(XD), TmmM,    REG(XS)))

#endif /* RT_SIMD_COMPAT_PW8 == 1 */

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorgx_rr(XG, XS)                                                    \
        xorgx3rr(W(XG), W(XG), W(XS))

#define xorgx_ld(XG, MS, DS)                                                \
        xorgx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorgx3rr(XD, XS, XT)                                                \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        notgx_rr(W(XG), W(XG))

#define notgx_rr(XD, XS)                                                    \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XS), REG(XS)))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
        addgx3rr(W(XG), W(XG), W(XS))

#define addgx_ld(XG, MS, DS)                                                \
        addgx3ld(W(XG), W(XG), W(MS), W(DS))

#define addgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000040 | MXM(REG(XD), REG(XS), REG(XT)))

#define addgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000040 | MXM(REG(XD), REG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgx_rr(XG, XS)                                                    \
        adsgx3rr(W(XG), W(XG), W(XS))

#define adsgx_ld(XG, MS, DS)                                                \
        adsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000240 | MXM(REG(XD), REG(XS), REG(XT)))

#define adsgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000240 | MXM(REG(XD), REG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgn_rr(XG, XS)                                                    \
        adsgn3rr(W(XG), W(XG), W(XS))

#define adsgn_ld(XG, MS, DS)                                                \
        adsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgn3rr(XD, XS, XT)                                                \
        EMITW(0x10000340 | MXM(REG(XD), REG(XS), REG(XT)))

#define adsgn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000340 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
        subgx3rr(W(XG), W(XG), W(XS))

#define subgx_ld(XG, MS, DS)                                                \
        subgx3ld(W(XG), W(XG), W(MS), W(DS))

#define subgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000440 | MXM(REG(XD), REG(XS), REG(XT)))

#define subgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000440 | MXM(REG(XD), REG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgx_rr(XG, XS)                                                    \
        sbsgx3rr(W(XG), W(XG), W(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
        sbsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000640 | MXM(REG(XD), REG(XS), REG(XT)))

#define sbsgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000640 | MXM(REG(XD), REG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgn_rr(XG, XS)                                                    \
        sbsgn3rr(W(XG), W(XG), W(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
        sbsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgn3rr(XD, XS, XT)                                                \
        EMITW(0x10000740 | MXM(REG(XD), REG(XS), REG(XT)))

#define sbsgn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000740 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
        mulgx3rr(W(XG), W(XG), W(XS))

#define mulgx_ld(XG, MS, DS)                                                \
        mulgx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgx3rr(XD, XS, XT)                                                \
        EMITW(0x100004C4 | MXM(TmmM,    TmmM,    TmmM))                     \
        EMITW(0x10000022 | MXM(REG(XD), REG(XS), REG(XT)) | TmmM << 6)

#define mulgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100004C4 | MXM(TmmQ,    TmmQ,    TmmQ))                     \
        EMITW(0x10000022 | MXM(REG(XD), REG(XS), TmmM) | TmmQ << 6)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)                                                    \
        shlgx3ri(W(XG), W(XG), W(IS))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgx3ri(XD, XS, IT)                                                \
        EMITW(0x1000034C | MXM(TmmM,    (0x0F & VAL(IT)), 0x00))            \
        EMITW(0x10000144 | MXM(REG(XD), REG(XS), TmmM))

#define shlgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00004E | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000024C | MXM(TmmM,    SP16,    TmmM))                     \
        EMITW(0x10000144 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)                                                    \
        shrgx3ri(W(XG), W(XG), W(IS))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgx3ri(XD, XS, IT)                                                \
        EMITW(0x1000034C | MXM(TmmM,    (0x0F & VAL(IT)), 0x00))            \
        EMITW(0x10000244 | MXM(REG(XD), REG(XS), TmmM))

#define shrgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00004E | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000024C | MXM(TmmM,    SP16,    TmmM))                     \
        EMITW(0x10000244 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)                                                    \
        shrgn3ri(W(XG), W(XG), W(IS))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgn3ri(XD, XS, IT)                                                \
        EMITW(0x1000034C | MXM(TmmM,    (0x0F & VAL(IT)), 0x00))            \
        EMITW(0x10000344 | MXM(REG(XD), REG(XS), TmmM))

#define shrgn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00004E | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000024C | MXM(TmmM,    SP16,    TmmM))                     \
        EMITW(0x10000344 | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx3rr(W(XG), W(XG), W(XS))

#define svlgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000144 | MXM(REG(XD), REG(XS), REG(XT)))

#define svlgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000144 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx3rr(W(XG), W(XG), W(XS))

#define svrgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000244 | MXM(REG(XD), REG(XS), REG(XT)))

#define svrgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000244 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn3rr(W(XG), W(XG), W(XS))

#define svrgn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgn3rr(XD, XS, XT)                                                \
        EMITW(0x10000344 | MXM(REG(XD), REG(XS), REG(XT)))

#define svrgn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000344 | MXM(REG(XD), REG(XS), TmmM))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
        mingx3rr(W(XG), W(XG), W(XS))

#define mingx_ld(XG, MS, DS)                                                \
        mingx3ld(W(XG), W(XG), W(MS), W(DS))

#define mingx3rr(XD, XS, XT)                                                \
        EMITW(0x10000242 | MXM(REG(XD), REG(XS), REG(XT)))

#define mingx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000242 | MXM(REG(XD), REG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingn_rr(XG, XS)                                                    \
        mingn3rr(W(XG), W(XG), W(XS))

#define mingn_ld(XG, MS, DS)                                                \
        mingn3ld(W(XG), W(XG), W(MS), W(DS))

#define mingn3rr(XD, XS, XT)                                                \
        EMITW(0x10000342 | MXM(REG(XD), REG(XS), REG(XT)))

#define mingn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000342 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
        maxgx3rr(W(XG), W(XG), W(XS))

#define maxgx_ld(XG, MS, DS)                                                \
        maxgx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000042 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000042 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgn_rr(XG, XS)                                                    \
        maxgn3rr(W(XG), W(XG), W(XS))

#define maxgn_ld(XG, MS, DS)                                                \
        maxgn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgn3rr(XD, XS, XT)                                                \
        EMITW(0x10000142 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxgn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000142 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgx_rr(XG, XS)                                                    \
        ceqgx3rr(W(XG), W(XG), W(XS))

#define ceqgx_ld(XG, MS, DS)                                                \
        ceqgx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000046 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000046 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegx_rr(XG, XS)                                                    \
        cnegx3rr(W(XG), W(XG), W(XS))

#define cnegx_ld(XG, MS, DS)                                                \
        cnegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegx3rr(XD, XS, XT)                                                \
        EMITW(0x10000046 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cnegx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000046 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgx_rr(XG, XS)                                                    \
        cltgx3rr(W(XG), W(XG), W(XS))

#define cltgx_ld(XG, MS, DS)                                                \
        cltgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000246 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000246 | MXM(REG(XD), TmmM,    REG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgn_rr(XG, XS)                                                    \
        cltgn3rr(W(XG), W(XG), W(XS))

#define cltgn_ld(XG, MS, DS)                                                \
        cltgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgn3rr(XD, XS, XT)                                                \
        EMITW(0x10000346 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltgn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000346 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegx_rr(XG, XS)                                                    \
        clegx3rr(W(XG), W(XG), W(XS))

#define clegx_ld(XG, MS, DS)                                                \
        clegx3ld(W(XG), W(XG), W(MS), W(DS))

#define clegx3rr(XD, XS, XT)                                                \
        EMITW(0x10000246 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define clegx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000246 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegn_rr(XG, XS)                                                    \
        clegn3rr(W(XG), W(XG), W(XS))

#define clegn_ld(XG, MS, DS)                                                \
        clegn3ld(W(XG), W(XG), W(MS), W(DS))

#define clegn3rr(XD, XS, XT)                                                \
        EMITW(0x10000346 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define clegn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000346 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgx_rr(XG, XS)                                                    \
        cgtgx3rr(W(XG), W(XG), W(XS))

#define cgtgx_ld(XG, MS, DS)                                                \
        cgtgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgx3rr(XD, XS, XT)                                                \
        EMITW(0x10000246 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtgx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000246 | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgn_rr(XG, XS)                                                    \
        cgtgn3rr(W(XG), W(XG), W(XS))

#define cgtgn_ld(XG, MS, DS)                                                \
        cgtgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgn3rr(XD, XS, XT)                                                \
        EMITW(0x10000346 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtgn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000346 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegx_rr(XG, XS)                                                    \
        cgegx3rr(W(XG), W(XG), W(XS))

#define cgegx_ld(XG, MS, DS)                                                \
        cgegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegx3rr(XD, XS, XT)                                                \
        EMITW(0x10000246 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cgegx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000246 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegn_rr(XG, XS)                                                    \
        cgegn3rr(W(XG), W(XG), W(XS))

#define cgegn_ld(XG, MS, DS)                                                \
        cgegn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegn3rr(XD, XS, XT)                                                \
        EMITW(0x10000346 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cgegn3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    Teax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000346 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_PHB_128X1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

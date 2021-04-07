/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_PQF_128X1V2_H
#define RT_RTARCH_PQF_128X1V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_pQF_128x1v2.h: Implementation of POWER fp128 VSX3 instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * and contains architecture-specific extensions
 * outside of the common assembler core.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdv*_rx - applies [cmd] to scalar-fp128: [r]egister (one operand)
 * cmdv*_rr - applies [cmd] to scalar-fp128: [r]egister from [r]egister
 *
 * cmdv*_rm - applies [cmd] to scalar-fp128: [r]egister from [m]emory
 * cmdv*_ld - applies [cmd] to scalar-fp128: as above (friendly alias)
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

#if (RT_128X1 == 2 || RT_128X1 == 8)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/****************   scalar quad-precision generic move/logic   ****************/

/* mov (D = S) */

#define movvx_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))

#define movvx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x00000000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movvx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x00000000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), O2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvvx_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))

#define mmvvx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))

#define mmvvx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), O2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andvx_rr(XG, XS)                                                    \
        andvx3rr(W(XG), W(XG), W(XS))

#define andvx_ld(XG, MS, DS)                                                \
        andvx3ld(W(XG), W(XG), W(MS), W(DS))

#define andvx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), REG(XT)))

#define andvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annvx_rr(XG, XS)                                                    \
        annvx3rr(W(XG), W(XG), W(XS))

#define annvx_ld(XG, MS, DS)                                                \
        annvx3ld(W(XG), W(XG), W(MS), W(DS))

#define annvx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000457 | MXM(REG(XD), REG(XT), REG(XS)))

#define annvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF0000457 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrvx_rr(XG, XS)                                                    \
        orrvx3rr(W(XG), W(XG), W(XS))

#define orrvx_ld(XG, MS, DS)                                                \
        orrvx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrvx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornvx_rr(XG, XS)                                                    \
        ornvx3rr(W(XG), W(XG), W(XS))

#define ornvx_ld(XG, MS, DS)                                                \
        ornvx3ld(W(XG), W(XG), W(MS), W(DS))

#define ornvx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000557 | MXM(REG(XD), REG(XT), REG(XS)))

#define ornvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF0000557 | MXM(REG(XD), TmmM,    REG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorvx_rr(XG, XS)                                                    \
        xorvx3rr(W(XG), W(XG), W(XS))

#define xorvx_ld(XG, MS, DS)                                                \
        xorvx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorvx3rr(XD, XS, XT)                                                \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notvx_rx(XG)                                                        \
        notvx_rr(W(XG), W(XG))

#define notvx_rr(XD, XS)                                                    \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XS), REG(XS)))

/*************   scalar quad-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negvs_rx(XG)                                                        \
        negvs_rr(W(XG), W(XG))

#define negvs_rr(XD, XS)                                                    \
        EMITW(0xFC000648 | MXM(REG(XD), 0x10,    REG(XS)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addvs_rr(XG, XS)                                                    \
        addvs3rr(W(XG), W(XG), W(XS))

#define addvs_ld(XG, MS, DS)                                                \
        addvs3ld(W(XG), W(XG), W(MS), W(DS))

#define addvs3rr(XD, XS, XT)                                                \
        EMITW(0xFC000008 | MXM(REG(XD), REG(XS), REG(XT)))

#define addvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xFC000008 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subvs_rr(XG, XS)                                                    \
        subvs3rr(W(XG), W(XG), W(XS))

#define subvs_ld(XG, MS, DS)                                                \
        subvs3ld(W(XG), W(XG), W(MS), W(DS))

#define subvs3rr(XD, XS, XT)                                                \
        EMITW(0xFC000408 | MXM(REG(XD), REG(XS), REG(XT)))

#define subvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xFC000408 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulvs_rr(XG, XS)                                                    \
        mulvs3rr(W(XG), W(XG), W(XS))

#define mulvs_ld(XG, MS, DS)                                                \
        mulvs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulvs3rr(XD, XS, XT)                                                \
        EMITW(0xFC000048 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xFC000048 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divvs_rr(XG, XS)                                                    \
        divvs3rr(W(XG), W(XG), W(XS))

#define divvs_ld(XG, MS, DS)                                                \
        divvs3ld(W(XG), W(XG), W(MS), W(DS))

#define divvs3rr(XD, XS, XT)                                                \
        EMITW(0xFC000448 | MXM(REG(XD), REG(XS), REG(XT)))

#define divvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xFC000448 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrvs_rr(XD, XS)                                                    \
        EMITW(0xFC000648 | MXM(REG(XD), 0x1B,    REG(XS)))

#define sqrvs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xFC000648 | MXM(REG(XD), 0x1B,    TmmM))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmavs_rr(XG, XS, XT)                                                \
        EMITW(0xFC000308 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmavs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xFC000308 | MXM(REG(XG), REG(XS), TmmM))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsvs_rr(XG, XS, XT)                                                \
        EMITW(0xFC0003C8 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsvs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xFC0003C8 | MXM(REG(XG), REG(XS), TmmM))

/*************   scalar quad-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addvx_rr(XG, XS)                                                    \
        addvx3rr(W(XG), W(XG), W(XS))

#define addvx_ld(XG, MS, DS)                                                \
        addvx3ld(W(XG), W(XG), W(MS), W(DS))

#define addvx3rr(XD, XS, XT)                                                \
        EMITW(0x10000100 | MXM(REG(XD), REG(XS), REG(XT)))

#define addvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x10000100 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subvx_rr(XG, XS)                                                    \
        subvx3rr(W(XG), W(XG), W(XS))

#define subvx_ld(XG, MS, DS)                                                \
        subvx3ld(W(XG), W(XG), W(MS), W(DS))

#define subvx3rr(XD, XS, XT)                                                \
        EMITW(0x10000500 | MXM(REG(XD), REG(XS), REG(XT)))

#define subvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x10000500 | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlvx_ri(XG, IS)                                                    \
        shlvx3ri(W(XG), W(XG), W(IS))

#define shlvx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlvx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlvx3ri(XD, XS, IT)                                                \
        EMITW(0xF00002D1 | TmmM << 21 | (0x7F & VAL(IT)) << 11)             \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))

#define shlvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrvx_ri(XG, IS)                                                    \
        shrvx3ri(W(XG), W(XG), W(IS))

#define shrvx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrvx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrvx3ri(XD, XS, IT)                                                \
        EMITW(0xF00002D1 | TmmM << 21 | (0x7F & VAL(IT)) << 11)             \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))

#define shrvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlvx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlvx3rr(W(XG), W(XG), W(XS))

#define svlvx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlvx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlvx3rr(XD, XS, XT)                                                \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    REG(XT)))                  \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))

#define svlvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrvx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrvx3rr(W(XG), W(XG), W(XS))

#define svrvx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrvx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrvx3rr(XD, XS, XT)                                                \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    REG(XT)))                  \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))

#define svrvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_PQF_128X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

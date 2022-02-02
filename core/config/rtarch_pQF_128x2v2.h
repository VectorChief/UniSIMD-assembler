/******************************************************************************/
/* Copyright (c) 2013-2022 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_PQF_128X2V2_H
#define RT_RTARCH_PQF_128X2V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_pQF_128x2v2.h: Implementation of POWER fp128 VSX3 instruction pairs.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * and contains architecture-specific extensions
 * outside of the common assembler core.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdu*_rx - applies [cmd] to packed-fp128: [r]egister (one operand)
 * cmdu*_rr - applies [cmd] to packed-fp128: [r]egister from [r]egister
 *
 * cmdu*_rm - applies [cmd] to packed-fp128: [r]egister from [m]emory
 * cmdu*_ld - applies [cmd] to packed-fp128: as above (friendly alias)
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing with 512-bit
 * and wider vectors. Use of scalars may leave respective vector registers
 * undefined, as seen from the perspective of any particular vector subset.
 *
 * 256-bit vectors used with wider subsets may not be compatible with regards
 * to memory loads/stores when mixed in the code. It means that data loaded
 * with wider vector and stored within 256-bit subset at the same address may
 * result in changing the initial representation in memory. The same can be
 * said about mixing vector and scalar subsets. Scalars can be completely
 * detached on some architectures. Use elm*x_st to store 1st vector element.
 * 128-bit vectors should be memory-compatible with any wider vector subset.
 *
 * Handling of NaNs in the floating point pipeline may not be consistent
 * across different architectures. Avoid NaNs entering the data flow by using
 * masking or control flow instructions. Apply special care when dealing with
 * floating point compare and min/max input/output. The result of floating point
 * compare instructions can be considered a -QNaN, though it is also interpreted
 * as integer -1 and is often treated as a mask. Most arithmetic instructions
 * should propagate QNaNs unchanged, however this behavior hasn't been tested.
 *
 * Note, that instruction subsets operating on vectors of different length
 * may support different number of SIMD registers, therefore mixing them
 * in the same code needs to be done with register awareness in mind.
 * For example, AVX-512 supports 32 SIMD registers, while AVX2 only has 16,
 * as does 256-bit paired subset on ARMv8, while 128-bit and SVE have 32.
 * These numbers should be consistent across architectures if properly
 * mapped to SIMD target mask presented in rtzero.h (compatibility layer).
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

#if (RT_128X2 == 2) && (RT_SIMD_COMPAT_XMM > 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed quad-precision generic move/logic   ****************/

/* mov (D = S) */

#define movux_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movux_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x00000000 | MPM(REG(XD), MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x00000000 | MPM(RYG(XD), MOD(MS), VYL(DS), B4(DS), L2(DS)))

#define movux_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMITW(0x00000000 | MPM(REG(XS), MOD(MD), VAL(DD), B4(DD), U2(DD)))  \
        EMITW(0x00000000 | MPM(RYG(XS), MOD(MD), VYL(DD), B4(DD), U2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvux_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mmvux_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), TmmM))

#define mmvux_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A2(DG), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), U2(DG)))  \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))  \
        EMITW(0xF000043F | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), U2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andux_rr(XG, XS)                                                    \
        andux3rr(W(XG), W(XG), W(XS))

#define andux_ld(XG, MS, DS)                                                \
        andux3ld(W(XG), W(XG), W(MS), W(DS))

#define andux3rr(XD, XS, XT)                                                \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annux_rr(XG, XS)                                                    \
        annux3rr(W(XG), W(XG), W(XS))

#define annux_ld(XG, MS, DS)                                                \
        annux3ld(W(XG), W(XG), W(MS), W(DS))

#define annux3rr(XD, XS, XT)                                                \
        EMITW(0xF0000457 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000457 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define annux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000457 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000457 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrux_rr(XG, XS)                                                    \
        orrux3rr(W(XG), W(XG), W(XS))

#define orrux_ld(XG, MS, DS)                                                \
        orrux3ld(W(XG), W(XG), W(MS), W(DS))

#define orrux3rr(XD, XS, XT)                                                \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornux_rr(XG, XS)                                                    \
        ornux3rr(W(XG), W(XG), W(XS))

#define ornux_ld(XG, MS, DS)                                                \
        ornux3ld(W(XG), W(XG), W(MS), W(DS))

#define ornux3rr(XD, XS, XT)                                                \
        EMITW(0xF0000557 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000557 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define ornux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000557 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF0000557 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorux_rr(XG, XS)                                                    \
        xorux3rr(W(XG), W(XG), W(XS))

#define xorux_ld(XG, MS, DS)                                                \
        xorux3ld(W(XG), W(XG), W(MS), W(DS))

#define xorux3rr(XD, XS, XT)                                                \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xorux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notux_rx(XG)                                                        \
        notux_rr(W(XG), W(XG))

#define notux_rr(XD, XS)                                                    \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XS), RYG(XS)))

/*************   packed quad-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negus_rx(XG)                                                        \
        negus_rr(W(XG), W(XG))

#define negus_rr(XD, XS)                                                    \
        EMITW(0xFC000648 | MXM(REG(XD), 0x10,    REG(XS)))                  \
        EMITW(0xFC000648 | MXM(RYG(XD), 0x10,    RYG(XS)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addus_rr(XG, XS)                                                    \
        addus3rr(W(XG), W(XG), W(XS))

#define addus_ld(XG, MS, DS)                                                \
        addus3ld(W(XG), W(XG), W(MS), W(DS))

#define addus3rr(XD, XS, XT)                                                \
        EMITW(0xFC000008 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xFC000008 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000008 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000008 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subus_rr(XG, XS)                                                    \
        subus3rr(W(XG), W(XG), W(XS))

#define subus_ld(XG, MS, DS)                                                \
        subus3ld(W(XG), W(XG), W(MS), W(DS))

#define subus3rr(XD, XS, XT)                                                \
        EMITW(0xFC000408 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xFC000408 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000408 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000408 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulus_rr(XG, XS)                                                    \
        mulus3rr(W(XG), W(XG), W(XS))

#define mulus_ld(XG, MS, DS)                                                \
        mulus3ld(W(XG), W(XG), W(MS), W(DS))

#define mulus3rr(XD, XS, XT)                                                \
        EMITW(0xFC000048 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xFC000048 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000048 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000048 | MXM(RYG(XD), RYG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divus_rr(XG, XS)                                                    \
        divus3rr(W(XG), W(XG), W(XS))

#define divus_ld(XG, MS, DS)                                                \
        divus3ld(W(XG), W(XG), W(MS), W(DS))

#define divus3rr(XD, XS, XT)                                                \
        EMITW(0xFC000448 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xFC000448 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000448 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000448 | MXM(RYG(XD), RYG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrus_rr(XD, XS)                                                    \
        EMITW(0xFC000648 | MXM(REG(XD), 0x1B,    REG(XS)))                  \
        EMITW(0xFC000648 | MXM(RYG(XD), 0x1B,    RYG(XS)))

#define sqrus_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0xFC000648 | MXM(REG(XD), 0x1B,    TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0xFC000648 | MXM(RYG(XD), 0x1B,    TmmM))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmaus_rr(XG, XS, XT)                                                \
        EMITW(0xFC000308 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xFC000308 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmaus_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000308 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC000308 | MXM(RYG(XG), RYG(XS), TmmM))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsus_rr(XG, XS, XT)                                                \
        EMITW(0xFC0003C8 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xFC0003C8 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsus_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC0003C8 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0xFC0003C8 | MXM(RYG(XG), RYG(XS), TmmM))

/*************   packed quad-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addux_rr(XG, XS)                                                    \
        addux3rr(W(XG), W(XG), W(XS))

#define addux_ld(XG, MS, DS)                                                \
        addux3ld(W(XG), W(XG), W(MS), W(DS))

#define addux3rr(XD, XS, XT)                                                \
        EMITW(0x10000100 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000100 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x10000100 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x10000100 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subux_rr(XG, XS)                                                    \
        subux3rr(W(XG), W(XG), W(XS))

#define subux_ld(XG, MS, DS)                                                \
        subux3ld(W(XG), W(XG), W(MS), W(DS))

#define subux3rr(XD, XS, XT)                                                \
        EMITW(0x10000500 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000500 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x10000500 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x10000500 | MXM(RYG(XD), RYG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlux_ri(XG, IS)                                                    \
        shlux3ri(W(XG), W(XG), W(IS))

#define shlux_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlux3ld(W(XG), W(XG), W(MS), W(DS))

#define shlux3ri(XD, XS, IT)                                                \
        EMITW(0xF00002D1 | TmmM << 21 | (0x7F & VAL(IT)) << 11)             \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x1000040C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(RYG(XD), RYG(XS), TmmM))

#define shlux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x1000040C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrux_ri(XG, IS)                                                    \
        shrux3ri(W(XG), W(XG), W(IS))

#define shrux_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrux3ld(W(XG), W(XG), W(MS), W(DS))

#define shrux3ri(XD, XS, IT)                                                \
        EMITW(0xF00002D1 | TmmM << 21 | (0x7F & VAL(IT)) << 11)             \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x1000044C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(RYG(XD), RYG(XS), TmmM))

#define shrux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x1000044C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(RYG(XD), RYG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlux_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlux3rr(W(XG), W(XG), W(XS))

#define svlux_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlux3ld(W(XG), W(XG), W(MS), W(DS))

#define svlux3rr(XD, XS, XT)                                                \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    REG(XT)))                  \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    RYG(XT)))                  \
        EMITW(0x1000040C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(RYG(XD), RYG(XS), TmmM))

#define svlux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000040C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000040C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100001C4 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrux_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrux3rr(W(XG), W(XG), W(XS))

#define svrux_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrux3ld(W(XG), W(XG), W(MS), W(DS))

#define svrux3rr(XD, XS, XT)                                                \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    REG(XT)))                  \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    RYG(XT)))                  \
        EMITW(0x1000044C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(RYG(XD), RYG(XS), TmmM))

#define svrux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000044C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x00000000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x1000020C | MXM(TmmM,    0x0F,    TmmM))                     \
        EMITW(0x1000044C | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x100002C4 | MXM(RYG(XD), RYG(XS), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_PQF_128X2V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

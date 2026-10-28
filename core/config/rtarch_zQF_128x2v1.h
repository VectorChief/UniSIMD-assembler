/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ZQF_128X2V1_H
#define RT_RTARCH_ZQF_128X2V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_zQF_128x2v1.h: Implementation of s390x fp128 SIMD instruction pairs.
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
 * cmd*x_** - applies [cmd] to SIMD/BASE unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to SIMD/BASE   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to SIMD/ELEM floating point   args, [s] - scalable
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

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

#if (defined RT_Z64)

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed quad-precision generic move/logic   ****************/

/* mov (D = S) */

#define movux_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))                     \
        EMIT6(MXM(0x56, RYG(XD),RYG(XS),   0x00, 0x00))

#define movux_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MPM(0x06, RYG(XD),MOD(MS),REG(MS), VYL(DS), B2(DS), L2(DS)))

#define movux_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), L2(DD)))  \
        EMIT6(MPM(0x0E, RYG(XS),MOD(MD),REG(MD), VYL(DD), B2(DD), L2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvux_rr(XG, XS)                                                    \
        andux_rr(W(XS), Xmm0)                                               \
        annux_rr(Xmm0, W(XG))                                               \
        orrux_rr(Xmm0, W(XS))                                               \
        movux_rr(W(XG), Xmm0)

#define mmvux_ld(XG, MS, DS)                                                \
        notux_rx(Xmm0)                                                      \
        andux_rr(W(XG), Xmm0)                                               \
        annux_ld(Xmm0, W(MS), W(DS))                                        \
        orrux_rr(W(XG), Xmm0)

#define mmvux_st(XS, MG, DG)                                                \
        andux_rr(W(XS), Xmm0)                                               \
        annux_ld(Xmm0, W(MG), W(DG))                                        \
        orrux_rr(Xmm0, W(XS))                                               \
        movux_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andux_rr(XG, XS)                                                    \
        andux3rr(W(XG), W(XG), W(XS))

#define andux_ld(XG, MS, DS)                                                \
        andux3ld(W(XG), W(XG), W(MS), W(DS))

#define andux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define andux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annux_rr(XG, XS)                                                    \
        annux3rr(W(XG), W(XG), W(XS))

#define annux_ld(XG, MS, DS)                                                \
        annux3ld(W(XG), W(XG), W(MS), W(DS))

#define annux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x69, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define annux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, RYG(XD),TmmM,   RYG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrux_rr(XG, XS)                                                    \
        orrux3rr(W(XG), W(XG), W(XS))

#define orrux_ld(XG, MS, DS)                                                \
        orrux3ld(W(XG), W(XG), W(MS), W(DS))

#define orrux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define orrux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornux_rr(XG, XS)                                                    \
        ornux3rr(W(XG), W(XG), W(XS))

#define ornux_ld(XG, MS, DS)                                                \
        ornux3ld(W(XG), W(XG), W(MS), W(DS))

#define ornux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6F, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define ornux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, RYG(XD),TmmM,   RYG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorux_rr(XG, XS)                                                    \
        xorux3rr(W(XG), W(XG), W(XS))

#define xorux_ld(XG, MS, DS)                                                \
        xorux3ld(W(XG), W(XG), W(MS), W(DS))

#define xorux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define xorux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notux_rx(XG)                                                        \
        notux_rr(W(XG), W(XG))

#define notux_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6B, RYG(XD),RYG(XS),RYG(XS), 0x00))

/*************   packed quad-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negus_rx(XG)                                                        \
        negus_rr(W(XG), W(XG))

#define negus_rr(XD, XS)                                                    \
        EMIT6(MYM(0xCC, REG(XD),REG(XS),   0x00, 0x04))                     \
        EMIT6(MYM(0xCC, RYG(XD),RYG(XS),   0x00, 0x04))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addus_rr(XG, XS)                                                    \
        addus3rr(W(XG), W(XG), W(XS))

#define addus_ld(XG, MS, DS)                                                \
        addus3ld(W(XG), W(XG), W(MS), W(DS))

#define addus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xE3, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define addus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE3, RYG(XD),RYG(XS),   TmmM, 0x04))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subus_rr(XG, XS)                                                    \
        subus3rr(W(XG), W(XG), W(XS))

#define subus_ld(XG, MS, DS)                                                \
        subus3ld(W(XG), W(XG), W(MS), W(DS))

#define subus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xE2, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define subus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE2, RYG(XD),RYG(XS),   TmmM, 0x04))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulus_rr(XG, XS)                                                    \
        mulus3rr(W(XG), W(XG), W(XS))

#define mulus_ld(XG, MS, DS)                                                \
        mulus3ld(W(XG), W(XG), W(MS), W(DS))

#define mulus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xE7, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define mulus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE7, RYG(XD),RYG(XS),   TmmM, 0x04))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divus_rr(XG, XS)                                                    \
        divus3rr(W(XG), W(XG), W(XS))

#define divus_ld(XG, MS, DS)                                                \
        divus3ld(W(XG), W(XG), W(MS), W(DS))

#define divus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xE5, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define divus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE5, RYG(XD),RYG(XS),   TmmM, 0x04))

/* sqr (D = sqrt S) */

#define sqrus_rr(XD, XS)                                                    \
        EMIT6(MWM(0xCE, REG(XD),REG(XS),   0x00, 0x04))                     \
        EMIT6(MWM(0xCE, RYG(XD),RYG(XS),   0x00, 0x04))

#define sqrus_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MWM(0xCE, REG(XD),TmmM,      0x00, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MWM(0xCE, RYG(XD),TmmM,      0x00, 0x04))

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaus_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MUM(0x8F, RYG(XG),RYG(XS),RYG(XT), 0x04))

#define fmaus_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MUM(0x8F, RYG(XG),RYG(XS),   TmmM, 0x04))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMA == 2

#define fmaus_rr(XG, XS, XT)                                                \
        movux_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulus_rr(W(XS), W(XT))                                              \
        addus_rr(W(XG), W(XS))                                              \
        movux_ld(W(XS), Mebp, inf_SCR01(0))

#define fmaus_ld(XG, XS, MT, DT)                                            \
        movux_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulus_ld(W(XS), W(MT), W(DT))                                       \
        addus_rr(W(XG), W(XS))                                              \
        movux_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#define fmaus3rr(XG, XS, XT)                                                \
        fmaus_rr(W(XG), W(XS), W(XT))

#define fmaus3ld(XG, XS, MT, DT)                                            \
        fmaus_ld(W(XG), W(XS), W(MT), W(DT))

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsus_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MUM(0x9E, RYG(XG),RYG(XS),RYG(XT), 0x04))

#define fmsus_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MUM(0x9E, RYG(XG),RYG(XS),   TmmM, 0x04))

#endif /* RT_SIMD_COMPAT_FMS */

#if RT_SIMD_COMPAT_FMS == 2

#define fmsus_rr(XG, XS, XT)                                                \
        movux_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulus_rr(W(XS), W(XT))                                              \
        subus_rr(W(XG), W(XS))                                              \
        movux_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsus_ld(XG, XS, MT, DT)                                            \
        movux_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulus_ld(W(XS), W(MT), W(DT))                                       \
        subus_rr(W(XG), W(XS))                                              \
        movux_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#define fmsus3rr(XG, XS, XT)                                                \
        fmsus_rr(W(XG), W(XS), W(XT))

#define fmsus3ld(XG, XS, MT, DT)                                            \
        fmsus_ld(W(XG), W(XS), W(MT), W(DT))

/**************   packed quad-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minus_rr(XG, XS)                                                    \
        minus3rr(W(XG), W(XG), W(XS))

#define minus_ld(XG, MS, DS)                                                \
        minus3ld(W(XG), W(XG), W(MS), W(DS))

#define minus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xEE, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define minus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEE, RYG(XD),RYG(XS),   TmmM, 0x04))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxus_rr(XG, XS)                                                    \
        maxus3rr(W(XG), W(XG), W(XS))

#define maxus_ld(XG, MS, DS)                                                \
        maxus3ld(W(XG), W(XG), W(MS), W(DS))

#define maxus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xEF, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define maxus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEF, RYG(XD),RYG(XS),   TmmM, 0x04))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define cequs_rr(XG, XS)                                                    \
        cequs3rr(W(XG), W(XG), W(XS))

#define cequs_ld(XG, MS, DS)                                                \
        cequs3ld(W(XG), W(XG), W(MS), W(DS))

#define cequs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define cequs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x04))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneus_rr(XG, XS)                                                    \
        cneus3rr(W(XG), W(XG), W(XS))

#define cneus_ld(XG, MS, DS)                                                \
        cneus3ld(W(XG), W(XG), W(MS), W(DS))

#define cneus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x04))                     \
        notux_rx(W(XD))

#define cneus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x04))                     \
        notux_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltus_rr(XG, XS)                                                    \
        cltus3rr(W(XG), W(XG), W(XS))

#define cltus_ld(XG, MS, DS)                                                \
        cltus3ld(W(XG), W(XG), W(MS), W(DS))

#define cltus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XT),REG(XS), 0x04))                     \
        EMIT6(MWM(0xEB, RYG(XD),RYG(XT),RYG(XS), 0x04))

#define cltus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),TmmM,   REG(XS), 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEB, RYG(XD),TmmM,   RYG(XS), 0x04))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleus_rr(XG, XS)                                                    \
        cleus3rr(W(XG), W(XG), W(XS))

#define cleus_ld(XG, MS, DS)                                                \
        cleus3ld(W(XG), W(XG), W(MS), W(DS))

#define cleus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XT),REG(XS), 0x04))                     \
        EMIT6(MWM(0xEA, RYG(XD),RYG(XT),RYG(XS), 0x04))

#define cleus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),TmmM,   REG(XS), 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEA, RYG(XD),TmmM,   RYG(XS), 0x04))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtus_rr(XG, XS)                                                    \
        cgtus3rr(W(XG), W(XG), W(XS))

#define cgtus_ld(XG, MS, DS)                                                \
        cgtus3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xEB, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define cgtus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEB, RYG(XD),RYG(XS),   TmmM, 0x04))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeus_rr(XG, XS)                                                    \
        cgeus3rr(W(XG), W(XG), W(XS))

#define cgeus_ld(XG, MS, DS)                                                \
        cgeus3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeus3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MWM(0xEA, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define cgeus3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MWM(0xEA, RYG(XD),RYG(XS),   TmmM, 0x04))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE00_256  MN00_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL00_256  MF00_256   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN00_256(xs, lb) /* not portable, do not use outside */            \
        EMIT6(MXM(0x6A, TmmM, xs, xs+16, 0x00))                             \
        EMIT6(MXM(0xD8, TmmM,TmmQ, 0x00, 0x00))                             \
        ASM_BEG ASM_OP1(je, lb) ASM_END

#define SMF00_256(xs, lb) /* not portable, do not use outside */            \
        EMIT6(MXM(0x68, TmmM, xs, xs+16, 0x00))                             \
        EMIT6(MXM(0xD8, TmmM,TmmQ, 0x00, 0x00))                             \
        ASM_BEG ASM_OP1(jo, lb) ASM_END

#define mkjux_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##00_256), EMPTY2)

/**************   packed quad-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit signed int range */

#define rnzus_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x05, 0x04))

#define rnzus_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x05, 0x04))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit signed int range */

#define rnpus_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x06, 0x04))

#define rnpus_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x06, 0x04))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit signed int range */

#define rnmus_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x07, 0x04))

#define rnmus_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x07, 0x04))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit signed int range */

#define rnnus_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x04, 0x04))

#define rnnus_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x04, 0x04))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit signed int range */

#define rndus_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x00, 0x04))

#define rndus_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x00, 0x04))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit signed int range */

#define rnrus_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x04))     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x04))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit unsigned int range */

#define ruzus_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x05, 0x04))

#define ruzus_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x05, 0x04))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit unsigned int range */

#define rupus_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x06, 0x04))

#define rupus_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x06, 0x04))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit unsigned int range */

#define rumus_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x07, 0x04))

#define rumus_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x07, 0x04))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit unsigned int range */

#define runus_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x04, 0x04))

#define runus_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x04, 0x04))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit unsigned int range */

#define rudus_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x04))                     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),   0x00, 0x04))

#define rudus_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MYM(0xC7, RYG(XD),TmmM,      0x00, 0x04))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: only round instructions are provided here, f128 SIMD fp-to-int
 * round instructions are only accurate within 128-bit unsigned int range */

#define rurus_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x04))     \
        EMIT6(MYM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x04))

/*************   packed quad-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addux_rr(XG, XS)                                                    \
        addux3rr(W(XG), W(XG), W(XS))

#define addux_ld(XG, MS, DS)                                                \
        addux3ld(W(XG), W(XG), W(MS), W(DS))

#define addux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define addux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),   TmmM, 0x04))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subux_rr(XG, XS)                                                    \
        subux3rr(W(XG), W(XG), W(XS))

#define subux_ld(XG, MS, DS)                                                \
        subux3ld(W(XG), W(XG), W(MS), W(DS))

#define subux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define subux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),   TmmM, 0x04))

#if RT_Z64 >= 17 /* only on z17 and beyond */

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulux_rr(XG, XS)                                                    \
        mulux3rr(W(XG), W(XG), W(XS))

#define mulux_ld(XG, MS, DS)                                                \
        mulux3ld(W(XG), W(XG), W(MS), W(DS))

#define mulux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xA2, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define mulux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xA2, RYG(XD),RYG(XS),   TmmM, 0x04))

/*****************   packed quad-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minux_rr(XG, XS)                                                    \
        minux3rr(W(XG), W(XG), W(XS))

#define minux_ld(XG, MS, DS)                                                \
        minux3ld(W(XG), W(XG), W(MS), W(DS))

#define minux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xFC, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define minux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFC, RYG(XD),RYG(XS),   TmmM, 0x04))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minun_rr(XG, XS)                                                    \
        minun3rr(W(XG), W(XG), W(XS))

#define minun_ld(XG, MS, DS)                                                \
        minun3ld(W(XG), W(XG), W(MS), W(DS))

#define minun3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xFE, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define minun3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFE, RYG(XD),RYG(XS),   TmmM, 0x04))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxux_rr(XG, XS)                                                    \
        maxux3rr(W(XG), W(XG), W(XS))

#define maxux_ld(XG, MS, DS)                                                \
        maxux3ld(W(XG), W(XG), W(MS), W(DS))

#define maxux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xFD, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define maxux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFD, RYG(XD),RYG(XS),   TmmM, 0x04))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxun_rr(XG, XS)                                                    \
        maxun3rr(W(XG), W(XG), W(XS))

#define maxun_ld(XG, MS, DS)                                                \
        maxun3ld(W(XG), W(XG), W(MS), W(DS))

#define maxun3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xFF, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define maxun3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFF, RYG(XD),RYG(XS),   TmmM, 0x04))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define cequx_rr(XG, XS)                                                    \
        cequx3rr(W(XG), W(XG), W(XS))

#define cequx_ld(XG, MS, DS)                                                \
        cequx3ld(W(XG), W(XG), W(MS), W(DS))

#define cequx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define cequx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x04))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneux_rr(XG, XS)                                                    \
        cneux3rr(W(XG), W(XG), W(XS))

#define cneux_ld(XG, MS, DS)                                                \
        cneux3ld(W(XG), W(XG), W(MS), W(DS))

#define cneux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x04))                     \
        notux_rx(W(XD))

#define cneux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x04))                     \
        notux_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltux_rr(XG, XS)                                                    \
        cltux3rr(W(XG), W(XG), W(XS))

#define cltux_ld(XG, MS, DS)                                                \
        cltux3ld(W(XG), W(XG), W(MS), W(DS))

#define cltux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XT),REG(XS), 0x04))                     \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XT),RYG(XS), 0x04))

#define cltux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),TmmM,   REG(XS), 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, RYG(XD),TmmM,   RYG(XS), 0x04))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltun_rr(XG, XS)                                                    \
        cltun3rr(W(XG), W(XG), W(XS))

#define cltun_ld(XG, MS, DS)                                                \
        cltun3ld(W(XG), W(XG), W(MS), W(DS))

#define cltun3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XT),REG(XS), 0x04))                     \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XT),RYG(XS), 0x04))

#define cltun3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),TmmM,   REG(XS), 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, RYG(XD),TmmM,   RYG(XS), 0x04))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleux_rr(XG, XS)                                                    \
        cleux3rr(W(XG), W(XG), W(XS))

#define cleux_ld(XG, MS, DS)                                                \
        cleux3ld(W(XG), W(XG), W(MS), W(DS))

#define cleux3rr(XD, XS, XT)                                                \
        cgtux3rr(W(XD), W(XS), W(XT))                                       \
        notux_rx(W(XD))

#define cleux3ld(XD, XS, MT, DT)                                            \
        cgtux3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notux_rx(W(XD))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleun_rr(XG, XS)                                                    \
        cleun3rr(W(XG), W(XG), W(XS))

#define cleun_ld(XG, MS, DS)                                                \
        cleun3ld(W(XG), W(XG), W(MS), W(DS))

#define cleun3rr(XD, XS, XT)                                                \
        cgtun3rr(W(XD), W(XS), W(XT))                                       \
        notux_rx(W(XD))

#define cleun3ld(XD, XS, MT, DT)                                            \
        cgtun3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notux_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtux_rr(XG, XS)                                                    \
        cgtux3rr(W(XG), W(XG), W(XS))

#define cgtux_ld(XG, MS, DS)                                                \
        cgtux3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtux3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define cgtux3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XS),   TmmM, 0x04))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtun_rr(XG, XS)                                                    \
        cgtun3rr(W(XG), W(XG), W(XS))

#define cgtun_ld(XG, MS, DS)                                                \
        cgtun3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtun3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),REG(XT), 0x04))                     \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XS),RYG(XT), 0x04))

#define cgtun3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),   TmmM, 0x04))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XS),   TmmM, 0x04))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeux_rr(XG, XS)                                                    \
        cgeux3rr(W(XG), W(XG), W(XS))

#define cgeux_ld(XG, MS, DS)                                                \
        cgeux3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeux3rr(XD, XS, XT)                                                \
        cltux3rr(W(XD), W(XS), W(XT))                                       \
        notux_rx(W(XD))

#define cgeux3ld(XD, XS, MT, DT)                                            \
        cltux3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notux_rx(W(XD))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeun_rr(XG, XS)                                                    \
        cgeun3rr(W(XG), W(XG), W(XS))

#define cgeun_ld(XG, MS, DS)                                                \
        cgeun3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeun3rr(XD, XS, XT)                                                \
        cltun3rr(W(XD), W(XS), W(XT))                                       \
        notux_rx(W(XD))

#define cgeun3ld(XD, XS, MT, DT)                                            \
        cltun3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notux_rx(W(XD))

#endif /* RT_Z64 >= 17 */

#endif /* (defined RT_Z64) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_ZQF_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

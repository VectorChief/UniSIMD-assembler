/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHB_SVEX2V1_H
#define RT_RTARCH_AHB_SVEX2V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHB_SVEx2v1.h: Implementation of AArch64 half+byte SVE pairs.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdm*_rx - applies [cmd] to var-len packed SIMD: [r]egister (one operand)
 * cmdm*_rr - applies [cmd] to var-len packed SIMD: [r]egister from [r]egister
 *
 * cmdm*_rm - applies [cmd] to var-len packed SIMD: [r]egister from [m]emory
 * cmdm*_ld - applies [cmd] to var-len packed SIMD: as above (friendly alias)
 *
 * cmdg*_** - applies [cmd] to 16-bit elements SIMD args, packed-128-bit
 * cmdgb_** - applies [cmd] to u-char elements SIMD args, packed-128-bit
 * cmdgc_** - applies [cmd] to s-char elements SIMD args, packed-128-bit
 *
 * cmda*_** - applies [cmd] to 16-bit elements SIMD args, packed-256-bit
 * cmdab_** - applies [cmd] to u-char elements SIMD args, packed-256-bit
 * cmdac_** - applies [cmd] to s-char elements SIMD args, packed-256-bit
 *
 * cmdn*_** - applies [cmd] to 16-bit elements ELEM args, scalar-fp-only
 * cmdh*_** - applies [cmd] to 16-bit elements BASE args, BASE-regs-only
 * cmdb*_** - applies [cmd] to  8-bit elements BASE args, BASE-regs-only
 *
 * cmd*x_** - applies [cmd] to SIMD/BASE unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to SIMD/BASE   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to SIMD/ELEM floating point   args, [s] - scalable
 *
 * The cmdm*_** (rtconf.h) instructions are intended for SPMD programming model
 * and simultaneously support 16/8-bit data elements (int, fp16 on ARM and x86).
 * In this model data paths are fixed-width, BASE and SIMD data elements are
 * width-compatible, code path divergence is handled via mkj**_** pseudo-ops.
 * Matching 16/8-bit BASE subsets cmdh* / cmdb* are defined in rtarch_*HB.h.
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

#if (RT_SVEX2 != 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmx_rr(XD, XS)                                                    \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0x04603000 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movmx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(REG(XD), MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x85804000 | MPM(RYG(XD), MOD(MS), VZL(DS), B3(DS), K1(DS)))

#define movmx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMITW(0xE5804000 | MPM(REG(XS), MOD(MD), VAL(DD), B3(DD), K1(DD)))  \
        EMITW(0xE5804000 | MPM(RYG(XS), MOD(MD), VZL(DD), B3(DD), K1(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvmx_rr(XG, XS)                                                    \
        EMITW(0x2440A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x0560C400 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x2440A000 | MXM(0x01,    Tmm0+16, TmmQ))                     \
        EMITW(0x0560C400 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define mmvmx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x2440A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x0560C400 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x2440A000 | MXM(0x01,    Tmm0+16, TmmQ))                     \
        EMITW(0x0560C400 | MXM(RYG(XG), TmmM,    RYG(XG)))

#define mmvmx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MG), VAL(DG), B3(DG), K1(DG)))  \
        EMITW(0x2440A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x0560C400 | MXM(TmmM,    REG(XS), TmmM))                     \
        EMITW(0xE5804000 | MPM(TmmM,    MOD(MG), VAL(DG), B3(DG), K1(DG)))  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MG), VZL(DG), B3(DG), K1(DG)))  \
        EMITW(0x2440A000 | MXM(0x01,    Tmm0+16, TmmQ))                     \
        EMITW(0x0560C400 | MXM(TmmM,    RYG(XS), TmmM))                     \
        EMITW(0xE5804000 | MPM(TmmM,    MOD(MG), VZL(DG), B3(DG), K1(DG)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andmx_rr(XG, XS)                                                    \
        andmx3rr(W(XG), W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andmx3ld(W(XG), W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        EMITW(0x04203000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04203000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04203000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04203000 | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annmx_rr(XG, XS)                                                    \
        annmx3rr(W(XG), W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        annmx3ld(W(XG), W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        EMITW(0x04E03000 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x04E03000 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define annmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04E03000 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04E03000 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrmx_rr(XG, XS)                                                    \
        orrmx3rr(W(XG), W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04603000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04603000 | MXM(RYG(XD), RYG(XS), TmmM))

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
        EMITW(0x04A03000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xormx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notmx_rx(XG)                                                        \
        notmx_rr(W(XG), W(XG))

#define notmx_rr(XD, XS)                                                    \
        EMITW(0x045EA000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x045EA000 | MXM(RYG(XD), RYG(XS), 0x00))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        EMITW(0x04600000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04600000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04600000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04600000 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsmx3rr(W(XG), W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        EMITW(0x04601400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04601400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsmn3rr(W(XG), W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        EMITW(0x04601000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04601000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601000 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        EMITW(0x04600400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04600400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define submx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04600400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04600400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsmx3rr(W(XG), W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        EMITW(0x04601C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04601C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsmn3rr(W(XG), W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        EMITW(0x04601800 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04601800 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601800 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04601800 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmx_rr(XG, XS)                                                    \
        EMITW(0x04500000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04500000 | MXM(RYG(XG), RYG(XS), 0x00))

#define mulmx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04500000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04500000 | MXM(RYG(XG), TmmM,    0x00))

#define mulmx3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        mulmx_rr(W(XD), W(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        mulmx_ld(W(XD), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)     /* emits shift-right with out-of-range args */ \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05222000 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x04538000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x04538000 | MXM(RYG(XG), TmmM,    0x00))

#define shlmx3ri(XD, XS, IT)                                                \
        EMITW(0x04309400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 16) & 0x00000800) | (M(VAL(IT) > 15) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F & VAL(IT)) << 16)))                       \
        EMITW(0x04309400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) < 16) & 0x00000800) | (M(VAL(IT) > 15) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F & VAL(IT)) << 16)))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        shlmx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05222000 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x04518000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x04518000 | MXM(RYG(XG), TmmM,    0x00))

#define shrmx3ri(XD, XS, IT)                                                \
        EMITW(0x04309400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000800) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))                       \
        EMITW(0x04309400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000800) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        shrmx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x7C400000 | MPM(TmmM,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05222000 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x04508000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x04508000 | MXM(RYG(XG), TmmM,    0x00))

#define shrmn3ri(XD, XS, IT)                                                \
        EMITW(0x04309000 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000C00) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))                       \
        EMITW(0x04309000 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000C00) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        shrmn_ld(W(XD), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04538000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04538000 | MXM(RYG(XG), RYG(XS), 0x00))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04538000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04538000 | MXM(RYG(XG), TmmM,    0x00))

#define svlmx3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        svlmx_rr(W(XD), W(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        svlmx_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04518000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04518000 | MXM(RYG(XG), RYG(XS), 0x00))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04518000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04518000 | MXM(RYG(XG), TmmM,    0x00))

#define svrmx3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        svrmx_rr(W(XD), W(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        svrmx_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04508000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04508000 | MXM(RYG(XG), RYG(XS), 0x00))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04508000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04508000 | MXM(RYG(XG), TmmM,    0x00))

#define svrmn3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        svrmn_rr(W(XD), W(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        svrmn_ld(W(XD), W(MT), W(DT))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        EMITW(0x044B0000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x044B0000 | MXM(RYG(XG), RYG(XS), 0x00))

#define minmx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x044B0000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x044B0000 | MXM(RYG(XG), TmmM,    0x00))

#define minmx3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        minmx_rr(W(XD), W(XT))

#define minmx3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        minmx_ld(W(XD), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        EMITW(0x044A0000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x044A0000 | MXM(RYG(XG), RYG(XS), 0x00))

#define minmn_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x044A0000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x044A0000 | MXM(RYG(XG), TmmM,    0x00))

#define minmn3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        minmn_rr(W(XD), W(XT))

#define minmn3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        minmn_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        EMITW(0x04490000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04490000 | MXM(RYG(XG), RYG(XS), 0x00))

#define maxmx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04490000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04490000 | MXM(RYG(XG), TmmM,    0x00))

#define maxmx3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        maxmx_rr(W(XD), W(XT))

#define maxmx3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        maxmx_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        EMITW(0x04480000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04480000 | MXM(RYG(XG), RYG(XS), 0x00))

#define maxmn_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04480000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04480000 | MXM(RYG(XG), TmmM,    0x00))

#define maxmn3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        maxmn_rr(W(XD), W(XT))

#define maxmn3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        maxmn_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqmx3rr(W(XG), W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqmx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        EMITW(0x2440A000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x2440A000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define ceqmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2440A000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2440A000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemx_rr(XG, XS)                                                    \
        cnemx3rr(W(XG), W(XG), W(XS))

#define cnemx_ld(XG, MS, DS)                                                \
        cnemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemx3rr(XD, XS, XT)                                                \
        EMITW(0x2440A010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x2440A010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cnemx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2440A010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2440A010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmx_rr(XG, XS)                                                    \
        cltmx3rr(W(XG), W(XG), W(XS))

#define cltmx_ld(XG, MS, DS)                                                \
        cltmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmx3rr(XD, XS, XT)                                                \
        EMITW(0x24400010 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24400010 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cltmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400010 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400010 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmn_rr(XG, XS)                                                    \
        cltmn3rr(W(XG), W(XG), W(XS))

#define cltmn_ld(XG, MS, DS)                                                \
        cltmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmn3rr(XD, XS, XT)                                                \
        EMITW(0x24408010 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24408010 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cltmn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408010 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408010 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemx_rr(XG, XS)                                                    \
        clemx3rr(W(XG), W(XG), W(XS))

#define clemx_ld(XG, MS, DS)                                                \
        clemx3ld(W(XG), W(XG), W(MS), W(DS))

#define clemx3rr(XD, XS, XT)                                                \
        EMITW(0x24400000 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24400000 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define clemx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400000 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400000 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemn_rr(XG, XS)                                                    \
        clemn3rr(W(XG), W(XG), W(XS))

#define clemn_ld(XG, MS, DS)                                                \
        clemn3ld(W(XG), W(XG), W(MS), W(DS))

#define clemn3rr(XD, XS, XT)                                                \
        EMITW(0x24408000 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24408000 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define clemn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408000 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408000 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmx_rr(XG, XS)                                                    \
        cgtmx3rr(W(XG), W(XG), W(XS))

#define cgtmx_ld(XG, MS, DS)                                                \
        cgtmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmx3rr(XD, XS, XT)                                                \
        EMITW(0x24400010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24400010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgtmx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtmn3rr(W(XG), W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        EMITW(0x24408010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24408010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgtmn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemx_rr(XG, XS)                                                    \
        cgemx3rr(W(XG), W(XG), W(XS))

#define cgemx_ld(XG, MS, DS)                                                \
        cgemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemx3rr(XD, XS, XT)                                                \
        EMITW(0x24400000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24400000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgemx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24400000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemn_rr(XG, XS)                                                    \
        cgemn3rr(W(XG), W(XG), W(XS))

#define cgemn_ld(XG, MS, DS)                                                \
        cgemn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemn3rr(XD, XS, XT)                                                \
        EMITW(0x24408000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24408000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgemn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24408000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_SVE     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_SVE     0x01    /*  all satisfy the condition */

#define mkjmx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x04203000 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                     (1 - RT_SIMD_MASK_##mask##16_SVE) << 22)               \
        EMITW(0x04582000 | MXM(TmmM,    TmmM,    0x00) |                    \
                          RT_SIMD_MASK_##mask##16_SVE << 17)                \
        EMITW(0x0E022C00 | MXM(TEax,    TmmM,    0x00))                     \
        addwxZri(Reax, IB(RT_SIMD_MASK_##mask##16_SVE))                     \
        jezxx_lb(lb)

/****************   packed byte-precision generic move/logic   ****************/

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvmb_rr(XG, XS)                                                    \
        EMITW(0x2400A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x0520C400 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x2400A000 | MXM(0x01,    Tmm0+16, TmmQ))                     \
        EMITW(0x0520C400 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define mmvmb_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x2400A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x0520C400 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x2400A000 | MXM(0x01,    Tmm0+16, TmmQ))                     \
        EMITW(0x0520C400 | MXM(RYG(XG), TmmM,    RYG(XG)))

#define mmvmb_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MG), VAL(DG), B3(DG), K1(DG)))  \
        EMITW(0x2400A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x0520C400 | MXM(TmmM,    REG(XS), TmmM))                     \
        EMITW(0xE5804000 | MPM(TmmM,    MOD(MG), VAL(DG), B3(DG), K1(DG)))  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MG), VZL(DG), B3(DG), K1(DG)))  \
        EMITW(0x2400A000 | MXM(0x01,    Tmm0+16, TmmQ))                     \
        EMITW(0x0520C400 | MXM(TmmM,    RYG(XS), TmmM))                     \
        EMITW(0xE5804000 | MPM(TmmM,    MOD(MG), VZL(DG), B3(DG), K1(DG)))

/* move/logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmb_rr(XG, XS)                                                    \
        addmb3rr(W(XG), W(XG), W(XS))

#define addmb_ld(XG, MS, DS)                                                \
        addmb3ld(W(XG), W(XG), W(MS), W(DS))

#define addmb3rr(XD, XS, XT)                                                \
        EMITW(0x04200000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04200000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addmb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04200000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04200000 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmb_rr(XG, XS)                                                    \
        adsmb3rr(W(XG), W(XG), W(XS))

#define adsmb_ld(XG, MS, DS)                                                \
        adsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmb3rr(XD, XS, XT)                                                \
        EMITW(0x04201400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04201400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsmb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmc_rr(XG, XS)                                                    \
        adsmc3rr(W(XG), W(XG), W(XS))

#define adsmc_ld(XG, MS, DS)                                                \
        adsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmc3rr(XD, XS, XT)                                                \
        EMITW(0x04201000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04201000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsmc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201000 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submb_rr(XG, XS)                                                    \
        submb3rr(W(XG), W(XG), W(XS))

#define submb_ld(XG, MS, DS)                                                \
        submb3ld(W(XG), W(XG), W(MS), W(DS))

#define submb3rr(XD, XS, XT)                                                \
        EMITW(0x04200400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04200400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define submb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04200400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04200400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmb_rr(XG, XS)                                                    \
        sbsmb3rr(W(XG), W(XG), W(XS))

#define sbsmb_ld(XG, MS, DS)                                                \
        sbsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmb3rr(XD, XS, XT)                                                \
        EMITW(0x04201C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04201C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsmb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmc_rr(XG, XS)                                                    \
        sbsmc3rr(W(XG), W(XG), W(XS))

#define sbsmc_ld(XG, MS, DS)                                                \
        sbsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmc3rr(XD, XS, XT)                                                \
        EMITW(0x04201800 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x04201800 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsmc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201800 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x04201800 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmb_rr(XG, XS)                                                    \
        EMITW(0x04100000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04100000 | MXM(RYG(XG), RYG(XS), 0x00))

#define mulmb_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04100000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04100000 | MXM(RYG(XG), TmmM,    0x00))

#define mulmb3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        mulmb_rr(W(XD), W(XT))

#define mulmb3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        mulmb_ld(W(XD), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmb_ri(XG, IS)     /* emits shift-right with out-of-range args */ \
        shlmb3ri(W(XG), W(XG), W(IS))

#define shlmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x05212000 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x04138000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x04138000 | MXM(RYG(XG), TmmM,    0x00))

#define shlmb3ri(XD, XS, IT)                                                \
        EMITW(0x04289400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 8) & 0x00000800) | (M(VAL(IT) > 7) & 0x00000000) |     \
        (M(VAL(IT) < 8) & ((0x07 & VAL(IT)) << 16)))                        \
        EMITW(0x04289400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) < 8) & 0x00000800) | (M(VAL(IT) > 7) & 0x00000000) |     \
        (M(VAL(IT) < 8) & ((0x07 & VAL(IT)) << 16)))

#define shlmb3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        shlmb_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmb_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrmb3ri(W(XG), W(XG), W(IS))

#define shrmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x05212000 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x04118000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x04118000 | MXM(RYG(XG), TmmM,    0x00))

#define shrmb3ri(XD, XS, IT)                                                \
        EMITW(0x04289400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000800) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))                        \
        EMITW(0x04289400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000800) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))

#define shrmb3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        shrmb_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmc_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrmc3ri(W(XG), W(XG), W(IS))

#define shrmc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x05212000 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x04108000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x04108000 | MXM(RYG(XG), TmmM,    0x00))

#define shrmc3ri(XD, XS, IT)                                                \
        EMITW(0x04289000 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000C00) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))                        \
        EMITW(0x04289000 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000C00) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))

#define shrmc3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        shrmc_ld(W(XD), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04138000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04138000 | MXM(RYG(XG), RYG(XS), 0x00))

#define svlmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04138000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04138000 | MXM(RYG(XG), TmmM,    0x00))

#define svlmb3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        svlmb_rr(W(XD), W(XT))

#define svlmb3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        svlmb_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04118000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04118000 | MXM(RYG(XG), RYG(XS), 0x00))

#define svrmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04118000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04118000 | MXM(RYG(XG), TmmM,    0x00))

#define svrmb3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        svrmb_rr(W(XD), W(XT))

#define svrmb3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        svrmb_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmc_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04108000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04108000 | MXM(RYG(XG), RYG(XS), 0x00))

#define svrmc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04108000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04108000 | MXM(RYG(XG), TmmM,    0x00))

#define svrmc3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        svrmc_rr(W(XD), W(XT))

#define svrmc3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        svrmc_ld(W(XD), W(MT), W(DT))

/*****************   packed byte-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmb_rr(XG, XS)                                                    \
        EMITW(0x040B0000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x040B0000 | MXM(RYG(XG), RYG(XS), 0x00))

#define minmb_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x040B0000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x040B0000 | MXM(RYG(XG), TmmM,    0x00))

#define minmb3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        minmb_rr(W(XD), W(XT))

#define minmb3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        minmb_ld(W(XD), W(MT), W(DT))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmc_rr(XG, XS)                                                    \
        EMITW(0x040A0000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x040A0000 | MXM(RYG(XG), RYG(XS), 0x00))

#define minmc_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x040A0000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x040A0000 | MXM(RYG(XG), TmmM,    0x00))

#define minmc3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        minmc_rr(W(XD), W(XT))

#define minmc3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        minmc_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmb_rr(XG, XS)                                                    \
        EMITW(0x04090000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04090000 | MXM(RYG(XG), RYG(XS), 0x00))

#define maxmb_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04090000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04090000 | MXM(RYG(XG), TmmM,    0x00))

#define maxmb3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        maxmb_rr(W(XD), W(XT))

#define maxmb3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        maxmb_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmc_rr(XG, XS)                                                    \
        EMITW(0x04080000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x04080000 | MXM(RYG(XG), RYG(XS), 0x00))

#define maxmc_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04080000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x04080000 | MXM(RYG(XG), TmmM,    0x00))

#define maxmc3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        maxmc_rr(W(XD), W(XT))

#define maxmc3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        maxmc_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmb_rr(XG, XS)                                                    \
        ceqmb3rr(W(XG), W(XG), W(XS))

#define ceqmb_ld(XG, MS, DS)                                                \
        ceqmb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmb3rr(XD, XS, XT)                                                \
        EMITW(0x2400A000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x2400A000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define ceqmb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2400A000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2400A000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemb_rr(XG, XS)                                                    \
        cnemb3rr(W(XG), W(XG), W(XS))

#define cnemb_ld(XG, MS, DS)                                                \
        cnemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemb3rr(XD, XS, XT)                                                \
        EMITW(0x2400A010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x2400A010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cnemb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2400A010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x2400A010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmb_rr(XG, XS)                                                    \
        cltmb3rr(W(XG), W(XG), W(XS))

#define cltmb_ld(XG, MS, DS)                                                \
        cltmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmb3rr(XD, XS, XT)                                                \
        EMITW(0x24000010 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24000010 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cltmb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000010 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000010 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmc_rr(XG, XS)                                                    \
        cltmc3rr(W(XG), W(XG), W(XS))

#define cltmc_ld(XG, MS, DS)                                                \
        cltmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmc3rr(XD, XS, XT)                                                \
        EMITW(0x24008010 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24008010 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cltmc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008010 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008010 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemb_rr(XG, XS)                                                    \
        clemb3rr(W(XG), W(XG), W(XS))

#define clemb_ld(XG, MS, DS)                                                \
        clemb3ld(W(XG), W(XG), W(MS), W(DS))

#define clemb3rr(XD, XS, XT)                                                \
        EMITW(0x24000000 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24000000 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define clemb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000000 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000000 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemc_rr(XG, XS)                                                    \
        clemc3rr(W(XG), W(XG), W(XS))

#define clemc_ld(XG, MS, DS)                                                \
        clemc3ld(W(XG), W(XG), W(MS), W(DS))

#define clemc3rr(XD, XS, XT)                                                \
        EMITW(0x24008000 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24008000 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define clemc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008000 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008000 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmb_rr(XG, XS)                                                    \
        cgtmb3rr(W(XG), W(XG), W(XS))

#define cgtmb_ld(XG, MS, DS)                                                \
        cgtmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmb3rr(XD, XS, XT)                                                \
        EMITW(0x24000010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24000010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgtmb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmc_rr(XG, XS)                                                    \
        cgtmc3rr(W(XG), W(XG), W(XS))

#define cgtmc_ld(XG, MS, DS)                                                \
        cgtmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmc3rr(XD, XS, XT)                                                \
        EMITW(0x24008010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24008010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgtmc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemb_rr(XG, XS)                                                    \
        cgemb3rr(W(XG), W(XG), W(XS))

#define cgemb_ld(XG, MS, DS)                                                \
        cgemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemb3rr(XD, XS, XT)                                                \
        EMITW(0x24000000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24000000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgemb3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24000000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemc_rr(XG, XS)                                                    \
        cgemc3rr(W(XG), W(XG), W(XS))

#define cgemc_ld(XG, MS, DS)                                                \
        cgemc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemc3rr(XD, XS, XT)                                                \
        EMITW(0x24008000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x24008000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgemc3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0520C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x24008000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0520C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_SVE     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_SVE     0x01    /*  all satisfy the condition */

#define mkjmb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x04203000 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                     (1 - RT_SIMD_MASK_##mask##08_SVE) << 22)               \
        EMITW(0x04182000 | MXM(TmmM,    TmmM,    0x00) |                    \
                          RT_SIMD_MASK_##mask##08_SVE << 17)                \
        EMITW(0x0E012C00 | MXM(TEax,    TmmM,    0x00))                     \
        addwxZri(Reax, IB(RT_SIMD_MASK_##mask##08_SVE))                     \
        jezxx_lb(lb)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_SVEX2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_AHB_SVEX2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

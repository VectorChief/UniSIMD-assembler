/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHF_SVEX2V1_H
#define RT_RTARCH_AHF_SVEX2V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHF_SVEx2v1.h: Implementation of AArch64 fp16 SVE instruction pairs.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * and contains architecture-specific extensions
 * outside of the common assembler core.
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

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmmx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movns_st(W(XS), W(MD), W(DD))

/*************   packed half-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negms_rx(XG)                                                        \
        negms_rr(W(XG), W(XG))

#define negms_rr(XD, XS)                                                    \
        EMITW(0x045DA000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x045DA000 | MXM(RYG(XD), RYG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addms_rr(XG, XS)                                                    \
        addms3rr(W(XG), W(XG), W(XS))

#define addms_ld(XG, MS, DS)                                                \
        addms3ld(W(XG), W(XG), W(MS), W(DS))

#define addms3rr(XD, XS, XT)                                                \
        EMITW(0x65400000 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x65400000 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addms3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65400000 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65400000 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subms_rr(XG, XS)                                                    \
        subms3rr(W(XG), W(XG), W(XS))

#define subms_ld(XG, MS, DS)                                                \
        subms3ld(W(XG), W(XG), W(MS), W(DS))

#define subms3rr(XD, XS, XT)                                                \
        EMITW(0x65400400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x65400400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subms3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65400400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65400400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulms_rr(XG, XS)                                                    \
        mulms3rr(W(XG), W(XG), W(XS))

#define mulms_ld(XG, MS, DS)                                                \
        mulms3ld(W(XG), W(XG), W(MS), W(DS))

#define mulms3rr(XD, XS, XT)                                                \
        EMITW(0x65400800 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x65400800 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulms3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65400800 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65400800 | MXM(RYG(XD), RYG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divms_rr(XG, XS)                                                    \
        EMITW(0x654D8000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x654D8000 | MXM(RYG(XG), RYG(XS), 0x00))

#define divms_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x654D8000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x654D8000 | MXM(RYG(XG), TmmM,    0x00))

#define divms3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        divms_rr(W(XD), W(XT))

#define divms3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        divms_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqrms_rr(XD, XS)                                                    \
        EMITW(0x654DA000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x654DA000 | MXM(RYG(XD), RYG(XS), 0x00))

#define sqrms_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x654DA000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x654DA000 | MXM(RYG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcems_rr(XD, XS)                                                    \
        EMITW(0x654E3000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x654E3000 | MXM(RYG(XD), RYG(XS), 0x00))

#define rcsms_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x65401800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65400800 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x65401800 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x65400800 | MXM(RYG(XG), RYG(XG), RYG(XS)))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsems_rr(XD, XS)                                                    \
        EMITW(0x654F3000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x654F3000 | MXM(RYG(XD), RYG(XS), 0x00))

#define rssms_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x65400800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65401C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65400800 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x65400800 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x65401C00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x65400800 | MXM(RYG(XG), RYG(XG), RYG(XS)))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmams_rr(XG, XS, XT)                                                \
        EMITW(0x65600000 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x65600000 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmams_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65600000 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65600000 | MXM(RYG(XG), RYG(XS), TmmM))

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsms_rr(XG, XS, XT)                                                \
        EMITW(0x65602000 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x65602000 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsms_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65602000 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65602000 | MXM(RYG(XG), RYG(XS), TmmM))

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minms_rr(XG, XS)                                                    \
        EMITW(0x65478000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x65478000 | MXM(RYG(XG), RYG(XS), 0x00))

#define minms_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x65478000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x65478000 | MXM(RYG(XG), TmmM,    0x00))

#define minms3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        minms_rr(W(XD), W(XT))

#define minms3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        minms_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxms_rr(XG, XS)                                                    \
        EMITW(0x65468000 | MXM(REG(XG), REG(XS), 0x00))                     \
        EMITW(0x65468000 | MXM(RYG(XG), RYG(XS), 0x00))

#define maxms_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x65468000 | MXM(REG(XG), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x65468000 | MXM(RYG(XG), TmmM,    0x00))

#define maxms3rr(XD, XS, XT)                                                \
        movmx_rr(W(XD), W(XS))                                              \
        maxms_rr(W(XD), W(XT))

#define maxms3ld(XD, XS, MT, DT)                                            \
        movmx_rr(W(XD), W(XS))                                              \
        maxms_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqms_rr(XG, XS)                                                    \
        ceqms3rr(W(XG), W(XG), W(XS))

#define ceqms_ld(XG, MS, DS)                                                \
        ceqms3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqms3rr(XD, XS, XT)                                                \
        EMITW(0x65406000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x65406000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define ceqms3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65406000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65406000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnems_rr(XG, XS)                                                    \
        cnems3rr(W(XG), W(XG), W(XS))

#define cnems_ld(XG, MS, DS)                                                \
        cnems3ld(W(XG), W(XG), W(MS), W(DS))

#define cnems3rr(XD, XS, XT)                                                \
        EMITW(0x65406010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x65406010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cnems3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65406010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65406010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltms_rr(XG, XS)                                                    \
        cltms3rr(W(XG), W(XG), W(XS))

#define cltms_ld(XG, MS, DS)                                                \
        cltms3ld(W(XG), W(XG), W(MS), W(DS))

#define cltms3rr(XD, XS, XT)                                                \
        EMITW(0x65404010 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x65404010 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cltms3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404010 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404010 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clems_rr(XG, XS)                                                    \
        clems3rr(W(XG), W(XG), W(XS))

#define clems_ld(XG, MS, DS)                                                \
        clems3ld(W(XG), W(XG), W(MS), W(DS))

#define clems3rr(XD, XS, XT)                                                \
        EMITW(0x65404000 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x65404000 | MXM(0x01,    RYG(XT), RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define clems3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404000 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404000 | MXM(0x01,    TmmM,    RYG(XS)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtms_rr(XG, XS)                                                    \
        cgtms3rr(W(XG), W(XG), W(XS))

#define cgtms_ld(XG, MS, DS)                                                \
        cgtms3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtms3rr(XD, XS, XT)                                                \
        EMITW(0x65404010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x65404010 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgtms3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404010 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgems_rr(XG, XS)                                                    \
        cgems3rr(W(XG), W(XG), W(XS))

#define cgems_ld(XG, MS, DS)                                                \
        cgems3ld(W(XG), W(XG), W(MS), W(DS))

#define cgems3rr(XD, XS, XT)                                                \
        EMITW(0x65404000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x65404000 | MXM(0x01,    RYG(XS), RYG(XT)))                  \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

#define cgems3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x0560C400 | MXM(REG(XD), TmmQ,    REG(XD)))                  \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VZL(DT), B3(DT), K1(DT)))  \
        EMITW(0x65404000 | MXM(0x01,    RYG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x0560C400 | MXM(RYG(XD), TmmQ,    RYG(XD)))

/* mkj (jump to lb) if (S satisfies mask condition) */

    /* mkj for half-precision is defined in corresponding HB_SVE header */

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzms_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x6543A000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6543A000 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnzms_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6543A000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6543A000 | MXM(RYG(XD), TmmM,    0x00))

#define cvzms_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x655AA000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x655AA000 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvzms_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x655AA000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x655AA000 | MXM(RYG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpms_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x6541A000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6541A000 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnpms_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6541A000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6541A000 | MXM(RYG(XD), TmmM,    0x00))

#define cvpms_rr(XD, XS)     /* round towards +inf */                       \
        rnpms_rr(W(XD), W(XS))                                              \
        cvzms_rr(W(XD), W(XD))

#define cvpms_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpms_ld(W(XD), W(MS), W(DS))                                       \
        cvzms_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmms_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x6542A000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6542A000 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnmms_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6542A000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6542A000 | MXM(RYG(XD), TmmM,    0x00))

#define cvmms_rr(XD, XS)     /* round towards -inf */                       \
        rnmms_rr(W(XD), W(XS))                                              \
        cvzms_rr(W(XD), W(XD))

#define cvmms_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmms_ld(W(XD), W(MS), W(DS))                                       \
        cvzms_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnnms_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x6540A000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6540A000 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnnms_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6540A000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6540A000 | MXM(RYG(XD), TmmM,    0x00))

#define cvnms_rr(XD, XS)     /* round towards near */                       \
        rnnms_rr(W(XD), W(XS))                                              \
        cvzms_rr(W(XD), W(XD))

#define cvnms_ld(XD, MS, DS) /* round towards near */                       \
        rnnms_ld(W(XD), W(MS), W(DS))                                       \
        cvzms_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnmn_rr(XD, XS)     /* round towards near */                       \
        cvtmn_rr(W(XD), W(XS))

#define cvnmn_ld(XD, MS, DS) /* round towards near */                       \
        cvtmn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndms_rr(XD, XS)                                                    \
        EMITW(0x6547A000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6547A000 | MXM(RYG(XD), RYG(XS), 0x00))

#define rndms_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6547A000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6547A000 | MXM(RYG(XD), TmmM,    0x00))

#define cvtms_rr(XD, XS)                                                    \
        rndms_rr(W(XD), W(XS))                                              \
        cvzms_rr(W(XD), W(XD))

#define cvtms_ld(XD, MS, DS)                                                \
        rndms_ld(W(XD), W(MS), W(DS))                                       \
        cvzms_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtmn_rr(XD, XS)                                                    \
        EMITW(0x6552A000 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6552A000 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtmn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6552A000 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VZL(DS), B3(DS), K1(DS)))  \
        EMITW(0x6552A000 | MXM(RYG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrms_rr(XD, XS, mode)                                              \
        EMITW(0x6540A000 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                        RT_SIMD_MODE_##mode << 16)          \
        EMITW(0x6540A000 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
                                        RT_SIMD_MODE_##mode << 16)

#define cvrms_rr(XD, XS, mode)                                              \
        rnrms_rr(W(XD), W(XS), mode)                                        \
        cvzms_rr(W(XD), W(XD))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_SVEX2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_AHF_SVEX2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

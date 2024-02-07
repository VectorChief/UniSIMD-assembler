/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHB_128X2V1_H
#define RT_RTARCH_AHB_128X2V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHB_128x2v1.h: Implementation of AArch64 half+byte NEON pairs.
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
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movax_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x3DC00000 | MPM(RYG(XD), MOD(MS), VYL(DS), B4(DS), L2(DS)))

#define movax_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DD), B4(DD), L2(DD)))  \
        EMITW(0x3D800000 | MPM(RYG(XS), MOD(MD), VYL(DD), B4(DD), L2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvax_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x6EA01C00 | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvax_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EA01C00 | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvax_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andax_rr(XG, XS)                                                    \
        andax3rr(W(XG), W(XG), W(XS))

#define andax_ld(XG, MS, DS)                                                \
        andax3ld(W(XG), W(XG), W(MS), W(DS))

#define andax3rr(XD, XS, XT)                                                \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E201C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E201C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annax_rr(XG, XS)                                                    \
        annax3rr(W(XG), W(XG), W(XS))

#define annax_ld(XG, MS, DS)                                                \
        annax3ld(W(XG), W(XG), W(MS), W(DS))

#define annax3rr(XD, XS, XT)                                                \
        EMITW(0x4E601C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4E601C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define annax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E601C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E601C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrax_rr(XG, XS)                                                    \
        orrax3rr(W(XG), W(XG), W(XS))

#define orrax_ld(XG, MS, DS)                                                \
        orrax3ld(W(XG), W(XG), W(MS), W(DS))

#define orrax3rr(XD, XS, XT)                                                \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornax_rr(XG, XS)                                                    \
        ornax3rr(W(XG), W(XG), W(XS))

#define ornax_ld(XG, MS, DS)                                                \
        ornax3ld(W(XG), W(XG), W(MS), W(DS))

#define ornax3rr(XD, XS, XT)                                                \
        EMITW(0x4EE01C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4EE01C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define ornax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE01C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE01C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorax_rr(XG, XS)                                                    \
        xorax3rr(W(XG), W(XG), W(XS))

#define xorax_ld(XG, MS, DS)                                                \
        xorax3ld(W(XG), W(XG), W(MS), W(DS))

#define xorax3rr(XD, XS, XT)                                                \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E201C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xorax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E201C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notax_rx(XG)                                                        \
        notax_rr(W(XG), W(XG))

#define notax_rr(XD, XS)                                                    \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XS), 0x00))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        EMITW(0x4E608400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E608400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E608400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E608400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
        adsax3rr(W(XG), W(XG), W(XS))

#define adsax_ld(XG, MS, DS)                                                \
        adsax3ld(W(XG), W(XG), W(MS), W(DS))

#define adsax3rr(XD, XS, XT)                                                \
        EMITW(0x6E600C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E600C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E600C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E600C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
        adsan3rr(W(XG), W(XG), W(XS))

#define adsan_ld(XG, MS, DS)                                                \
        adsan3ld(W(XG), W(XG), W(MS), W(DS))

#define adsan3rr(XD, XS, XT)                                                \
        EMITW(0x4E600C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E600C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsan3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E600C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E600C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        EMITW(0x6E608400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E608400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E608400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E608400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
        sbsax3rr(W(XG), W(XG), W(XS))

#define sbsax_ld(XG, MS, DS)                                                \
        sbsax3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsax3rr(XD, XS, XT)                                                \
        EMITW(0x6E602C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E602C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E602C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E602C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
        sbsan3rr(W(XG), W(XG), W(XS))

#define sbsan_ld(XG, MS, DS)                                                \
        sbsan3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsan3rr(XD, XS, XT)                                                \
        EMITW(0x4E602C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E602C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsan3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E602C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E602C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        EMITW(0x4E609C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E609C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E609C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E609C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)     /* emits shift-right with out-of-range args */ \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        EMITW(0x4F100400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 16) & 0x00005000) | (M(VAL(IT) > 15) & 0x20000000) |   \
        (M(VAL(IT) < 16) & ((0x0F & VAL(IT)) << 16)))                       \
        EMITW(0x4F100400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) < 16) & 0x00005000) | (M(VAL(IT) > 15) & 0x20000000) |   \
        (M(VAL(IT) < 16) & ((0x0F & VAL(IT)) << 16)))

#define shlax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E020400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E604400 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        EMITW(0x4F100400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))                       \
        EMITW(0x4F100400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))

#define shrax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E020400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E604400 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        EMITW(0x4F100400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))                       \
        EMITW(0x4F100400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 16) & ((0x0F &-VAL(IT)) << 16)))

#define shran3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E020400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x4E604400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlax3rr(W(XG), W(XG), W(XS))

#define svlax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlax3ld(W(XG), W(XG), W(MS), W(DS))

#define svlax3rr(XD, XS, XT)                                                \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E604400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svlax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E604400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrax3rr(W(XG), W(XG), W(XS))

#define svrax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrax3ld(W(XG), W(XG), W(MS), W(DS))

#define svrax3rr(XD, XS, XT)                                                \
        EMITW(0x6E60B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E60B800 | MXM(TmmM,    RYG(XT), 0x00))                     \
        EMITW(0x6E604400 | MXM(RYG(XD), RYG(XS), TmmM))

#define svrax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E604400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svran_rr(XG, XS)     /* variable shift with per-elem count */       \
        svran3rr(W(XG), W(XG), W(XS))

#define svran_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svran3ld(W(XG), W(XG), W(MS), W(DS))

#define svran3rr(XD, XS, XT)                                                \
        EMITW(0x6E60B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x4E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E60B800 | MXM(TmmM,    RYG(XT), 0x00))                     \
        EMITW(0x4E604400 | MXM(RYG(XD), RYG(XS), TmmM))

#define svran3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E604400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E604400 | MXM(RYG(XD), RYG(XS), TmmM))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minax_rr(XG, XS)                                                    \
        minax3rr(W(XG), W(XG), W(XS))

#define minax_ld(XG, MS, DS)                                                \
        minax3ld(W(XG), W(XG), W(MS), W(DS))

#define minax3rr(XD, XS, XT)                                                \
        EMITW(0x6E606C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E606C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E606C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E606C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minan_rr(XG, XS)                                                    \
        minan3rr(W(XG), W(XG), W(XS))

#define minan_ld(XG, MS, DS)                                                \
        minan3ld(W(XG), W(XG), W(MS), W(DS))

#define minan3rr(XD, XS, XT)                                                \
        EMITW(0x4E606C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E606C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minan3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E606C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E606C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxax_rr(XG, XS)                                                    \
        maxax3rr(W(XG), W(XG), W(XS))

#define maxax_ld(XG, MS, DS)                                                \
        maxax3ld(W(XG), W(XG), W(MS), W(DS))

#define maxax3rr(XD, XS, XT)                                                \
        EMITW(0x6E606400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E606400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E606400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E606400 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxan_rr(XG, XS)                                                    \
        maxan3rr(W(XG), W(XG), W(XS))

#define maxan_ld(XG, MS, DS)                                                \
        maxan3ld(W(XG), W(XG), W(MS), W(DS))

#define maxan3rr(XD, XS, XT)                                                \
        EMITW(0x4E606400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E606400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxan3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E606400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E606400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqax_rr(XG, XS)                                                    \
        ceqax3rr(W(XG), W(XG), W(XS))

#define ceqax_ld(XG, MS, DS)                                                \
        ceqax3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqax3rr(XD, XS, XT)                                                \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E608C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E608C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneax_rr(XG, XS)                                                    \
        cneax3rr(W(XG), W(XG), W(XS))

#define cneax_ld(XG, MS, DS)                                                \
        cneax3ld(W(XG), W(XG), W(MS), W(DS))

#define cneax3rr(XD, XS, XT)                                                \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x6E608C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

#define cneax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E608C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E608C00 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltax_rr(XG, XS)                                                    \
        cltax3rr(W(XG), W(XG), W(XS))

#define cltax_ld(XG, MS, DS)                                                \
        cltax3ld(W(XG), W(XG), W(MS), W(DS))

#define cltax3rr(XD, XS, XT)                                                \
        EMITW(0x6E603400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6E603400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltan_rr(XG, XS)                                                    \
        cltan3rr(W(XG), W(XG), W(XS))

#define cltan_ld(XG, MS, DS)                                                \
        cltan3ld(W(XG), W(XG), W(MS), W(DS))

#define cltan3rr(XD, XS, XT)                                                \
        EMITW(0x4E603400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4E603400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltan3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleax_rr(XG, XS)                                                    \
        cleax3rr(W(XG), W(XG), W(XS))

#define cleax_ld(XG, MS, DS)                                                \
        cleax3ld(W(XG), W(XG), W(MS), W(DS))

#define cleax3rr(XD, XS, XT)                                                \
        EMITW(0x6E603C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6E603C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clean_rr(XG, XS)                                                    \
        clean3rr(W(XG), W(XG), W(XS))

#define clean_ld(XG, MS, DS)                                                \
        clean3ld(W(XG), W(XG), W(MS), W(DS))

#define clean3rr(XD, XS, XT)                                                \
        EMITW(0x4E603C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4E603C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define clean3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtax_rr(XG, XS)                                                    \
        cgtax3rr(W(XG), W(XG), W(XS))

#define cgtax_ld(XG, MS, DS)                                                \
        cgtax3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtax3rr(XD, XS, XT)                                                \
        EMITW(0x6E603400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E603400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtan_rr(XG, XS)                                                    \
        cgtan3rr(W(XG), W(XG), W(XS))

#define cgtan_ld(XG, MS, DS)                                                \
        cgtan3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtan3rr(XD, XS, XT)                                                \
        EMITW(0x4E603400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E603400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtan3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeax_rr(XG, XS)                                                    \
        cgeax3rr(W(XG), W(XG), W(XS))

#define cgeax_ld(XG, MS, DS)                                                \
        cgeax3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeax3rr(XD, XS, XT)                                                \
        EMITW(0x6E603C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E603C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeax3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E603C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgean_rr(XG, XS)                                                    \
        cgean3rr(W(XG), W(XG), W(XS))

#define cgean_ld(XG, MS, DS)                                                \
        cgean3ld(W(XG), W(XG), W(MS), W(DS))

#define cgean3rr(XD, XS, XT)                                                \
        EMITW(0x4E603C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E603C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgean3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E603C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_256     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_256     0x08    /*  all satisfy the condition */

#define mkjax_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4E201C00 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                  (0x08 - RT_SIMD_MASK_##mask##16_256) << 20)               \
        EMITW(0x4E71B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x0E022C00 | MXM(TEax,    TmmM,    0x00))                     \
        addwxZri(Reax, IB(RT_SIMD_MASK_##mask##16_256))                     \
        jezxx_lb(lb)

/****************   packed byte-precision generic move/logic   ****************/

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvab_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x6EA01C00 | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvab_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EA01C00 | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvab_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))

/* move/logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addab_rr(XG, XS)                                                    \
        addab3rr(W(XG), W(XG), W(XS))

#define addab_ld(XG, MS, DS)                                                \
        addab3ld(W(XG), W(XG), W(MS), W(DS))

#define addab3rr(XD, XS, XT)                                                \
        EMITW(0x4E208400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E208400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E208400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E208400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsab_rr(XG, XS)                                                    \
        adsab3rr(W(XG), W(XG), W(XS))

#define adsab_ld(XG, MS, DS)                                                \
        adsab3ld(W(XG), W(XG), W(MS), W(DS))

#define adsab3rr(XD, XS, XT)                                                \
        EMITW(0x6E200C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E200C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E200C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E200C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsac_rr(XG, XS)                                                    \
        adsac3rr(W(XG), W(XG), W(XS))

#define adsac_ld(XG, MS, DS)                                                \
        adsac3ld(W(XG), W(XG), W(MS), W(DS))

#define adsac3rr(XD, XS, XT)                                                \
        EMITW(0x4E200C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E200C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adsac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E200C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E200C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subab_rr(XG, XS)                                                    \
        subab3rr(W(XG), W(XG), W(XS))

#define subab_ld(XG, MS, DS)                                                \
        subab3ld(W(XG), W(XG), W(MS), W(DS))

#define subab3rr(XD, XS, XT)                                                \
        EMITW(0x6E208400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E208400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E208400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E208400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsab_rr(XG, XS)                                                    \
        sbsab3rr(W(XG), W(XG), W(XS))

#define sbsab_ld(XG, MS, DS)                                                \
        sbsab3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsab3rr(XD, XS, XT)                                                \
        EMITW(0x6E202C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E202C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E202C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E202C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsac_rr(XG, XS)                                                    \
        sbsac3rr(W(XG), W(XG), W(XS))

#define sbsac_ld(XG, MS, DS)                                                \
        sbsac3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsac3rr(XD, XS, XT)                                                \
        EMITW(0x4E202C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E202C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define sbsac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E202C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E202C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulab_rr(XG, XS)                                                    \
        mulab3rr(W(XG), W(XG), W(XS))

#define mulab_ld(XG, MS, DS)                                                \
        mulab3ld(W(XG), W(XG), W(MS), W(DS))

#define mulab3rr(XD, XS, XT)                                                \
        EMITW(0x4E209C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E209C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E209C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E209C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlab_ri(XG, IS)     /* emits shift-right with out-of-range args */ \
        shlab3ri(W(XG), W(XG), W(IS))

#define shlab_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlab3ld(W(XG), W(XG), W(MS), W(DS))

#define shlab3ri(XD, XS, IT)                                                \
        EMITW(0x4F080400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 8) & 0x00005000) | (M(VAL(IT) > 7) & 0x20000000) |     \
        (M(VAL(IT) < 8) & ((0x07 & VAL(IT)) << 16)))                        \
        EMITW(0x4F080400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) < 8) & 0x00005000) | (M(VAL(IT) > 7) & 0x20000000) |     \
        (M(VAL(IT) < 8) & ((0x07 & VAL(IT)) << 16)))

#define shlab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E010400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E204400 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrab_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrab3ri(W(XG), W(XG), W(IS))

#define shrab_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrab3ld(W(XG), W(XG), W(MS), W(DS))

#define shrab3ri(XD, XS, IT)                                                \
        EMITW(0x4F080400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))                        \
        EMITW(0x4F080400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))

#define shrab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E010400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E204400 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrac_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrac3ri(W(XG), W(XG), W(IS))

#define shrac_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrac3ld(W(XG), W(XG), W(MS), W(DS))

#define shrac3ri(XD, XS, IT)                                                \
        EMITW(0x4F080400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))                        \
        EMITW(0x4F080400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 8) & ((0x07 &-VAL(IT)) << 16)))

#define shrac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E010400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x4E204400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlab_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlab3rr(W(XG), W(XG), W(XS))

#define svlab_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlab3ld(W(XG), W(XG), W(MS), W(DS))

#define svlab3rr(XD, XS, XT)                                                \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E204400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svlab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E204400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrab_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrab3rr(W(XG), W(XG), W(XS))

#define svrab_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrab3ld(W(XG), W(XG), W(MS), W(DS))

#define svrab3rr(XD, XS, XT)                                                \
        EMITW(0x6E20B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E20B800 | MXM(TmmM,    RYG(XT), 0x00))                     \
        EMITW(0x6E204400 | MXM(RYG(XD), RYG(XS), TmmM))

#define svrab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6E204400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrac_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrac3rr(W(XG), W(XG), W(XS))

#define svrac_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrac3ld(W(XG), W(XG), W(MS), W(DS))

#define svrac3rr(XD, XS, XT)                                                \
        EMITW(0x6E20B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x4E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E20B800 | MXM(TmmM,    RYG(XT), 0x00))                     \
        EMITW(0x4E204400 | MXM(RYG(XD), RYG(XS), TmmM))

#define svrac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E204400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E20B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4E204400 | MXM(RYG(XD), RYG(XS), TmmM))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minab_rr(XG, XS)                                                    \
        minab3rr(W(XG), W(XG), W(XS))

#define minab_ld(XG, MS, DS)                                                \
        minab3ld(W(XG), W(XG), W(MS), W(DS))

#define minab3rr(XD, XS, XT)                                                \
        EMITW(0x6E206C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E206C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E206C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E206C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minac_rr(XG, XS)                                                    \
        minac3rr(W(XG), W(XG), W(XS))

#define minac_ld(XG, MS, DS)                                                \
        minac3ld(W(XG), W(XG), W(MS), W(DS))

#define minac3rr(XD, XS, XT)                                                \
        EMITW(0x4E206C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E206C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E206C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E206C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxab_rr(XG, XS)                                                    \
        maxab3rr(W(XG), W(XG), W(XS))

#define maxab_ld(XG, MS, DS)                                                \
        maxab3ld(W(XG), W(XG), W(MS), W(DS))

#define maxab3rr(XD, XS, XT)                                                \
        EMITW(0x6E206400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E206400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E206400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E206400 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxac_rr(XG, XS)                                                    \
        maxac3rr(W(XG), W(XG), W(XS))

#define maxac_ld(XG, MS, DS)                                                \
        maxac3ld(W(XG), W(XG), W(MS), W(DS))

#define maxac3rr(XD, XS, XT)                                                \
        EMITW(0x4E206400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E206400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E206400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E206400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqab_rr(XG, XS)                                                    \
        ceqab3rr(W(XG), W(XG), W(XS))

#define ceqab_ld(XG, MS, DS)                                                \
        ceqab3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqab3rr(XD, XS, XT)                                                \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E208C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E208C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneab_rr(XG, XS)                                                    \
        cneab3rr(W(XG), W(XG), W(XS))

#define cneab_ld(XG, MS, DS)                                                \
        cneab3ld(W(XG), W(XG), W(MS), W(DS))

#define cneab3rr(XD, XS, XT)                                                \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x6E208C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

#define cneab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E208C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E208C00 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltab_rr(XG, XS)                                                    \
        cltab3rr(W(XG), W(XG), W(XS))

#define cltab_ld(XG, MS, DS)                                                \
        cltab3ld(W(XG), W(XG), W(MS), W(DS))

#define cltab3rr(XD, XS, XT)                                                \
        EMITW(0x6E203400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6E203400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltac_rr(XG, XS)                                                    \
        cltac3rr(W(XG), W(XG), W(XS))

#define cltac_ld(XG, MS, DS)                                                \
        cltac3ld(W(XG), W(XG), W(MS), W(DS))

#define cltac3rr(XD, XS, XT)                                                \
        EMITW(0x4E203400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4E203400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleab_rr(XG, XS)                                                    \
        cleab3rr(W(XG), W(XG), W(XS))

#define cleab_ld(XG, MS, DS)                                                \
        cleab3ld(W(XG), W(XG), W(MS), W(DS))

#define cleab3rr(XD, XS, XT)                                                \
        EMITW(0x6E203C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6E203C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleac_rr(XG, XS)                                                    \
        cleac3rr(W(XG), W(XG), W(XS))

#define cleac_ld(XG, MS, DS)                                                \
        cleac3ld(W(XG), W(XG), W(MS), W(DS))

#define cleac3rr(XD, XS, XT)                                                \
        EMITW(0x4E203C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4E203C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtab_rr(XG, XS)                                                    \
        cgtab3rr(W(XG), W(XG), W(XS))

#define cgtab_ld(XG, MS, DS)                                                \
        cgtab3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtab3rr(XD, XS, XT)                                                \
        EMITW(0x6E203400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E203400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtac_rr(XG, XS)                                                    \
        cgtac3rr(W(XG), W(XG), W(XS))

#define cgtac_ld(XG, MS, DS)                                                \
        cgtac3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtac3rr(XD, XS, XT)                                                \
        EMITW(0x4E203400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E203400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeab_rr(XG, XS)                                                    \
        cgeab3rr(W(XG), W(XG), W(XS))

#define cgeab_ld(XG, MS, DS)                                                \
        cgeab3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeab3rr(XD, XS, XT)                                                \
        EMITW(0x6E203C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E203C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeab3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E203C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeac_rr(XG, XS)                                                    \
        cgeac3rr(W(XG), W(XG), W(XS))

#define cgeac_ld(XG, MS, DS)                                                \
        cgeac3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeac3rr(XD, XS, XT)                                                \
        EMITW(0x4E203C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E203C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeac3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E203C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_256     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_256     0x10    /*  all satisfy the condition */

#define mkjab_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4E201C00 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                  (0x10 - RT_SIMD_MASK_##mask##08_256) << 19)               \
        EMITW(0x4E31B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x0E012C00 | MXM(TEax,    TmmM,    0x00))                     \
        addwxZri(Reax, IB(RT_SIMD_MASK_##mask##08_256))                     \
        jezxx_lb(lb)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_AHB_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

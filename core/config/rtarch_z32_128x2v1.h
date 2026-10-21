/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_Z32_128X2V1_H
#define RT_RTARCH_Z32_128X2V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_z32_128x*v1.h: Implementation of s390x SIMD instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdp*_ri - applies [cmd] to [p]acked: [r]egister from [i]mmediate
 * cmdp*_rr - applies [cmd] to [p]acked: [r]egister from [r]egister
 *
 * cmdp*_rm - applies [cmd] to [p]acked: [r]egister from [m]emory
 * cmdp*_ld - applies [cmd] to [p]acked: as above
 *
 * cmdi*_** - applies [cmd] to 32-bit elements SIMD args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit elements SIMD args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size elements SIMD args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit elements SIMD args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit elements SIMD args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size elements SIMD args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit elements SIMD args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size elements SIMD args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit elements SIMD args, packed-var-len
 *
 * cmdr*_** - applies [cmd] to 32-bit elements ELEM args, scalar-fp-only
 * cmds*_** - applies [cmd] to L-size elements ELEM args, scalar-fp-only
 * cmdt*_** - applies [cmd] to 64-bit elements ELEM args, scalar-fp-only
 *
 * cmd*x_** - applies [cmd] to SIMD/BASE unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to SIMD/BASE   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to SIMD/ELEM floating point   args, [s] - scalable
 *
 * The cmdp*_** (rtconf.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data elements (fp+int).
 * In this model data paths are fixed-width, BASE and SIMD data elements are
 * width-compatible, code path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtconf.h as well.
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

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmcx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmix_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movcx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))                     \
        EMIT6(MXM(0x56, RYG(XD),RYG(XS),   0x00, 0x00))

#define movcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MPM(0x06, RYG(XD),MOD(MS),REG(MS), VYL(DS), B2(DS), L2(DS)))

#define movcx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), L2(DD)))  \
        EMIT6(MPM(0x0E, RYG(XS),MOD(MD),REG(MD), VYL(DD), B2(DD), L2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvcx_rr(XG, XS)                                                    \
        andcx_rr(W(XS), Xmm0)                                               \
        anncx_rr(Xmm0, W(XG))                                               \
        orrcx_rr(Xmm0, W(XS))                                               \
        movcx_rr(W(XG), Xmm0)

#define mmvcx_ld(XG, MS, DS)                                                \
        notcx_rx(Xmm0)                                                      \
        andcx_rr(W(XG), Xmm0)                                               \
        anncx_ld(Xmm0, W(MS), W(DS))                                        \
        orrcx_rr(W(XG), Xmm0)

#define mmvcx_st(XS, MG, DG)                                                \
        andcx_rr(W(XS), Xmm0)                                               \
        anncx_ld(Xmm0, W(MG), W(DG))                                        \
        orrcx_rr(Xmm0, W(XS))                                               \
        movcx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andcx_rr(XG, XS)                                                    \
        andcx3rr(W(XG), W(XG), W(XS))

#define andcx_ld(XG, MS, DS)                                                \
        andcx3ld(W(XG), W(XG), W(MS), W(DS))

#define andcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define andcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anncx_rr(XG, XS)                                                    \
        anncx3rr(W(XG), W(XG), W(XS))

#define anncx_ld(XG, MS, DS)                                                \
        anncx3ld(W(XG), W(XG), W(MS), W(DS))

#define anncx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x69, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define anncx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, RYG(XD),TmmM,   RYG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrcx_rr(XG, XS)                                                    \
        orrcx3rr(W(XG), W(XG), W(XS))

#define orrcx_ld(XG, MS, DS)                                                \
        orrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define orrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orncx_rr(XG, XS)                                                    \
        orncx3rr(W(XG), W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        orncx3ld(W(XG), W(XG), W(MS), W(DS))

#define orncx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6F, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define orncx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, RYG(XD),TmmM,   RYG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorcx_rr(XG, XS)                                                    \
        xorcx3rr(W(XG), W(XG), W(XS))

#define xorcx_ld(XG, MS, DS)                                                \
        xorcx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define xorcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notcx_rx(XG)                                                        \
        notcx_rr(W(XG), W(XG))

#define notcx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6B, RYG(XD),RYG(XS),RYG(XS), 0x00))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negcs_rx(XG)                                                        \
        xorcx_ld(W(XG), Mebp, inf_GPC06_32)

#define negcs_rr(XD, XS)                                                    \
        movcx_rr(W(XD), W(XS))                                              \
        negcs_rx(W(XD))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcs_rr(XG, XS)                                                    \
        addcs3rr(W(XG), W(XG), W(XS))

#define addcs_ld(XG, MS, DS)                                                \
        addcs3ld(W(XG), W(XG), W(MS), W(DS))

#define addcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define addcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcs_rr(XG, XS)                                                    \
        subcs3rr(W(XG), W(XG), W(XS))

#define subcs_ld(XG, MS, DS)                                                \
        subcs3ld(W(XG), W(XG), W(MS), W(DS))

#define subcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define subcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),   TmmM, 0x02))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcs_rr(XG, XS)                                                    \
        mulcs3rr(W(XG), W(XG), W(XS))

#define mulcs_ld(XG, MS, DS)                                                \
        mulcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mulcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divcs_rr(XG, XS)                                                    \
        divcs3rr(W(XG), W(XG), W(XS))

#define divcs_ld(XG, MS, DS)                                                \
        divcs3ld(W(XG), W(XG), W(MS), W(DS))

#define divcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define divcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),   TmmM, 0x02))

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        EMIT6(MXM(0xCE, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MXM(0xCE, RYG(XD),RYG(XS),   0x00, 0x02))

#define sqrcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, RYG(XD),TmmM,      0x00, 0x02))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),RYG(XT), 0x02))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),RYG(XT), 0x02))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),   TmmM, 0x02))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mincs_rr(XG, XS)                                                    \
        mincs3rr(W(XG), W(XG), W(XS))

#define mincs_ld(XG, MS, DS)                                                \
        mincs3ld(W(XG), W(XG), W(MS), W(DS))

#define mincs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mincs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxcs_rr(XG, XS)                                                    \
        maxcs3rr(W(XG), W(XG), W(XS))

#define maxcs_ld(XG, MS, DS)                                                \
        maxcs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define maxcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcs_rr(XG, XS)                                                    \
        ceqcs3rr(W(XG), W(XG), W(XS))

#define ceqcs_ld(XG, MS, DS)                                                \
        ceqcs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define ceqcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecs_rr(XG, XS)                                                    \
        cnecs3rr(W(XG), W(XG), W(XS))

#define cnecs_ld(XG, MS, DS)                                                \
        cnecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x02))                     \
        notcx_rx(W(XD))

#define cnecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x02))                     \
        notcx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltcs_rr(XG, XS)                                                    \
        cltcs3rr(W(XG), W(XG), W(XS))

#define cltcs_ld(XG, MS, DS)                                                \
        cltcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XT),REG(XS), 0x02))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XT),RYG(XS), 0x02))

#define cltcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),TmmM,   REG(XS), 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),TmmM,   RYG(XS), 0x02))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clecs_rr(XG, XS)                                                    \
        clecs3rr(W(XG), W(XG), W(XS))

#define clecs_ld(XG, MS, DS)                                                \
        clecs3ld(W(XG), W(XG), W(MS), W(DS))

#define clecs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XT),REG(XS), 0x02))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XT),RYG(XS), 0x02))

#define clecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),TmmM,   REG(XS), 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),TmmM,   RYG(XS), 0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtcs_rr(XG, XS)                                                    \
        cgtcs3rr(W(XG), W(XG), W(XS))

#define cgtcs_ld(XG, MS, DS)                                                \
        cgtcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define cgtcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgecs_rr(XG, XS)                                                    \
        cgecs3rr(W(XG), W(XG), W(XS))

#define cgecs_ld(XG, MS, DS)                                                \
        cgecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecs3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define cgecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),   TmmM, 0x02))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_256  MN32_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256  MF32_256   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN32_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0,  0, lb) ASM_END

#define SMF32_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0, -1, lb) ASM_END

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMIT6(MXM(0x94, TmmM,   REG(XS),RYG(XS), 0x02))                     \
        EMIT6(MXM(0x94, TmmM,   TmmM,   TmmM,    0x01))                     \
        EMIT6(MXM(0x21, TMxx,   TmmM,   0x00,    0x03))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##32_256), EMPTY2)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x02))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x02))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x05, 0x02))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x05, 0x02))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x02))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x02))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x06, 0x02))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x06, 0x02))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x02))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x02))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x07, 0x02))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x07, 0x02))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x02))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x02))

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x04, 0x02))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x04, 0x02))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x02))

#define rndcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x02))

#define cvtcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x00, 0x02))

#define cvtcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x00, 0x02))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define cvrcs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x04, 0x02))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x04, 0x02))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcn_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x00, 0x02))

#define cvtcn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x00, 0x02))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncx_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x04, 0x02))

#define cvncx_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x04, 0x02))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcx_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x00, 0x02))

#define cvtcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x00, 0x02))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x02))

#define ruzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x02))

#define cuzcs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x05, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x05, 0x02))

#define cuzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x05, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x05, 0x02))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x02))

#define rupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x02))

#define cupcs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x06, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x06, 0x02))

#define cupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x06, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x06, 0x02))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x02))

#define rumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x02))

#define cumcs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x07, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x07, 0x02))

#define cumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x07, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x07, 0x02))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runcs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x02))

#define runcs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x02))

#define cuncs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x04, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x04, 0x02))

#define cuncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x04, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x04, 0x02))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x02))

#define rudcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x02))

#define cutcs_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x00, 0x02))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x00, 0x02))

#define cutcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x00, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x00, 0x02))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurcs_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x02))

#define curcs_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x02))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcx_rr(XG, XS)                                                    \
        addcx3rr(W(XG), W(XG), W(XS))

#define addcx_ld(XG, MS, DS)                                                \
        addcx3ld(W(XG), W(XG), W(MS), W(DS))

#define addcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define addcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),   TmmM, 0x02))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcx_rr(XG, XS)                                                    \
        subcx3rr(W(XG), W(XG), W(XS))

#define subcx_ld(XG, MS, DS)                                                \
        subcx3ld(W(XG), W(XG), W(MS), W(DS))

#define subcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define subcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),   TmmM, 0x02))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcx_rr(XG, XS)                                                    \
        mulcx3rr(W(XG), W(XG), W(XS))

#define mulcx_ld(XG, MS, DS)                                                \
        mulcx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xA2, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mulcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xA2, RYG(XD),RYG(XS),   TmmM, 0x02))

        /* div, rem are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlcx_ri(XG, IS)                                                    \
        shlcx3ri(W(XG), W(XG), W(IS))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlcx3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x14), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x1C), W(IT))                             \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcx_ri(XG, IS)                                                    \
        shrcx3ri(W(XG), W(XG), W(IS))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcx3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x14), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x1C), W(IT))                             \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcn_ri(XG, IS)                                                    \
        shrcn3ri(W(XG), W(XG), W(IS))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcn3ri(XD, XS, IT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x14), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x1C), W(IT))                             \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrcn3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx3rr(W(XG), W(XG), W(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlcx_rx(W(XD))

#define svlcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlcx_rx(W(XD))

#define svlcx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx3rr(W(XG), W(XG), W(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrcx_rx(W(XD))

#define svrcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrcx_rx(W(XD))

#define svrcx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn3rr(W(XG), W(XG), W(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcn3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrcn_rx(W(XD))

#define svrcn3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrcn_rx(W(XD))

#define svrcn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mincx_rr(XG, XS)                                                    \
        mincx3rr(W(XG), W(XG), W(XS))

#define mincx_ld(XG, MS, DS)                                                \
        mincx3ld(W(XG), W(XG), W(MS), W(DS))

#define mincx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xFC, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mincx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFC, RYG(XD),RYG(XS),   TmmM, 0x02))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mincn_rr(XG, XS)                                                    \
        mincn3rr(W(XG), W(XG), W(XS))

#define mincn_ld(XG, MS, DS)                                                \
        mincn3ld(W(XG), W(XG), W(MS), W(DS))

#define mincn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xFE, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define mincn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFE, RYG(XD),RYG(XS),   TmmM, 0x02))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxcx_rr(XG, XS)                                                    \
        maxcx3rr(W(XG), W(XG), W(XS))

#define maxcx_ld(XG, MS, DS)                                                \
        maxcx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xFD, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define maxcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFD, RYG(XD),RYG(XS),   TmmM, 0x02))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxcn_rr(XG, XS)                                                    \
        maxcn3rr(W(XG), W(XG), W(XS))

#define maxcn_ld(XG, MS, DS)                                                \
        maxcn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xFF, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define maxcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFF, RYG(XD),RYG(XS),   TmmM, 0x02))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcx_rr(XG, XS)                                                    \
        ceqcx3rr(W(XG), W(XG), W(XS))

#define ceqcx_ld(XG, MS, DS)                                                \
        ceqcx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define ceqcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecx_rr(XG, XS)                                                    \
        cnecx3rr(W(XG), W(XG), W(XS))

#define cnecx_ld(XG, MS, DS)                                                \
        cnecx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x02))                     \
        notcx_rx(W(XD))

#define cnecx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x02))                     \
        notcx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltcx_rr(XG, XS)                                                    \
        cltcx3rr(W(XG), W(XG), W(XS))

#define cltcx_ld(XG, MS, DS)                                                \
        cltcx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XT),REG(XS), 0x02))                     \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XT),RYG(XS), 0x02))

#define cltcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),TmmM,   REG(XS), 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, RYG(XD),TmmM,   RYG(XS), 0x02))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltcn_rr(XG, XS)                                                    \
        cltcn3rr(W(XG), W(XG), W(XS))

#define cltcn_ld(XG, MS, DS)                                                \
        cltcn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XT),REG(XS), 0x02))                     \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XT),RYG(XS), 0x02))

#define cltcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),TmmM,   REG(XS), 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, RYG(XD),TmmM,   RYG(XS), 0x02))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clecx_rr(XG, XS)                                                    \
        clecx3rr(W(XG), W(XG), W(XS))

#define clecx_ld(XG, MS, DS)                                                \
        clecx3ld(W(XG), W(XG), W(MS), W(DS))

#define clecx3rr(XD, XS, XT)                                                \
        cgtcx3rr(W(XD), W(XS), W(XT))                                       \
        notcx_rx(W(XD))

#define clecx3ld(XD, XS, MT, DT)                                            \
        cgtcx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notcx_rx(W(XD))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clecn_rr(XG, XS)                                                    \
        clecn3rr(W(XG), W(XG), W(XS))

#define clecn_ld(XG, MS, DS)                                                \
        clecn3ld(W(XG), W(XG), W(MS), W(DS))

#define clecn3rr(XD, XS, XT)                                                \
        cgtcn3rr(W(XD), W(XS), W(XT))                                       \
        notcx_rx(W(XD))

#define clecn3ld(XD, XS, MT, DT)                                            \
        cgtcn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notcx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtcx_rr(XG, XS)                                                    \
        cgtcx3rr(W(XG), W(XG), W(XS))

#define cgtcx_ld(XG, MS, DS)                                                \
        cgtcx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define cgtcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtcn_rr(XG, XS)                                                    \
        cgtcn3rr(W(XG), W(XG), W(XS))

#define cgtcn_ld(XG, MS, DS)                                                \
        cgtcn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),REG(XT), 0x02))                     \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XS),RYG(XT), 0x02))

#define cgtcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),   TmmM, 0x02))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XS),   TmmM, 0x02))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgecx_rr(XG, XS)                                                    \
        cgecx3rr(W(XG), W(XG), W(XS))

#define cgecx_ld(XG, MS, DS)                                                \
        cgecx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecx3rr(XD, XS, XT)                                                \
        cltcx3rr(W(XD), W(XS), W(XT))                                       \
        notcx_rx(W(XD))

#define cgecx3ld(XD, XS, MT, DT)                                            \
        cltcx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notcx_rx(W(XD))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgecn_rr(XG, XS)                                                    \
        cgecn3rr(W(XG), W(XG), W(XS))

#define cgecn_ld(XG, MS, DS)                                                \
        cgecn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecn3rr(XD, XS, XT)                                                \
        cltcn3rr(W(XD), W(XS), W(XT))                                       \
        notcx_rx(W(XD))

#define cgecn3ld(XD, XS, MT, DT)                                            \
        cltcn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notcx_rx(W(XD))

#endif /* (defined RT_Z64) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_Z32_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

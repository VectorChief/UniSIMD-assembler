/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_Z64_128X2V1_H
#define RT_RTARCH_Z64_128X2V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_z64_128x*v1.h: Implementation of s390x SIMD instructions.
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

#define elmdx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmjx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movdx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))                     \
        EMIT6(MXM(0x56, RYG(XD),RYG(XS),   0x00, 0x00))

#define movdx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MPM(0x06, RYG(XD),MOD(MS),REG(MS), VYL(DS), B2(DS), L2(DS)))

#define movdx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), L2(DD)))  \
        EMIT6(MPM(0x0E, RYG(XS),MOD(MD),REG(MD), VYL(DD), B2(DD), L2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvdx_rr(XG, XS)                                                    \
        anddx_rr(W(XS), Xmm0)                                               \
        anndx_rr(Xmm0, W(XG))                                               \
        orrdx_rr(Xmm0, W(XS))                                               \
        movdx_rr(W(XG), Xmm0)

#define mmvdx_ld(XG, MS, DS)                                                \
        notdx_rx(Xmm0)                                                      \
        anddx_rr(W(XG), Xmm0)                                               \
        anndx_ld(Xmm0, W(MS), W(DS))                                        \
        orrdx_rr(W(XG), Xmm0)

#define mmvdx_st(XS, MG, DG)                                                \
        anddx_rr(W(XS), Xmm0)                                               \
        anndx_ld(Xmm0, W(MG), W(DG))                                        \
        orrdx_rr(Xmm0, W(XS))                                               \
        movdx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define anddx_rr(XG, XS)                                                    \
        anddx3rr(W(XG), W(XG), W(XS))

#define anddx_ld(XG, MS, DS)                                                \
        anddx3ld(W(XG), W(XG), W(MS), W(DS))

#define anddx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define anddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x68, RYG(XD),RYG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anndx_rr(XG, XS)                                                    \
        anndx3rr(W(XG), W(XG), W(XS))

#define anndx_ld(XG, MS, DS)                                                \
        anndx3ld(W(XG), W(XG), W(MS), W(DS))

#define anndx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x69, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define anndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x69, RYG(XD),TmmM,   RYG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrdx_rr(XG, XS)                                                    \
        orrdx3rr(W(XG), W(XG), W(XS))

#define orrdx_ld(XG, MS, DS)                                                \
        orrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define orrdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6A, RYG(XD),RYG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orndx_rr(XG, XS)                                                    \
        orndx3rr(W(XG), W(XG), W(XS))

#define orndx_ld(XG, MS, DS)                                                \
        orndx3ld(W(XG), W(XG), W(MS), W(DS))

#define orndx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6F, RYG(XD),RYG(XT),RYG(XS), 0x00))

#define orndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6F, RYG(XD),TmmM,   RYG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xordx_rr(XG, XS)                                                    \
        xordx3rr(W(XG), W(XG), W(XS))

#define xordx_ld(XG, MS, DS)                                                \
        xordx3ld(W(XG), W(XG), W(MS), W(DS))

#define xordx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))                     \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),RYG(XT), 0x00))

#define xordx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0x6D, RYG(XD),RYG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notdx_rx(XG)                                                        \
        notdx_rr(W(XG), W(XG))

#define notdx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))                     \
        EMIT6(MXM(0x6B, RYG(XD),RYG(XS),RYG(XS), 0x00))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negds_rx(XG)                                                        \
        xordx_ld(W(XG), Mebp, inf_GPC06_64)

#define negds_rr(XD, XS)                                                    \
        movdx_rr(W(XD), W(XS))                                              \
        negds_rx(W(XD))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addds_rr(XG, XS)                                                    \
        addds3rr(W(XG), W(XG), W(XS))

#define addds_ld(XG, MS, DS)                                                \
        addds3ld(W(XG), W(XG), W(MS), W(DS))

#define addds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define addds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE3, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subds_rr(XG, XS)                                                    \
        subds3rr(W(XG), W(XG), W(XS))

#define subds_ld(XG, MS, DS)                                                \
        subds3ld(W(XG), W(XG), W(MS), W(DS))

#define subds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define subds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE2, RYG(XD),RYG(XS),   TmmM, 0x03))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulds_rr(XG, XS)                                                    \
        mulds3rr(W(XG), W(XG), W(XS))

#define mulds_ld(XG, MS, DS)                                                \
        mulds3ld(W(XG), W(XG), W(MS), W(DS))

#define mulds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define mulds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE7, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divds_rr(XG, XS)                                                    \
        divds3rr(W(XG), W(XG), W(XS))

#define divds_ld(XG, MS, DS)                                                \
        divds3ld(W(XG), W(XG), W(MS), W(DS))

#define divds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define divds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE5, RYG(XD),RYG(XS),   TmmM, 0x03))

/* sqr (D = sqrt S) */

#define sqrds_rr(XD, XS)                                                    \
        EMIT6(MXM(0xCE, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MXM(0xCE, RYG(XD),RYG(XS),   0x00, 0x03))

#define sqrds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MXM(0xCE, RYG(XD),TmmM,      0x00, 0x03))

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

#define fmads_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),RYG(XT), 0x03))

#define fmads_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, REG(XG),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x8F, RYG(XG),RYG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsds_rr(XG, XS, XT)                                                \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),RYG(XT), 0x03))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, REG(XG),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MVM(0x9E, RYG(XG),RYG(XS),   TmmM, 0x03))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minds_rr(XG, XS)                                                    \
        minds3rr(W(XG), W(XG), W(XS))

#define minds_ld(XG, MS, DS)                                                \
        minds3ld(W(XG), W(XG), W(MS), W(DS))

#define minds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define minds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEE, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxds_rr(XG, XS)                                                    \
        maxds3rr(W(XG), W(XG), W(XS))

#define maxds_ld(XG, MS, DS)                                                \
        maxds3ld(W(XG), W(XG), W(MS), W(DS))

#define maxds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define maxds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEF, RYG(XD),RYG(XS),   TmmM, 0x03))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqds_rr(XG, XS)                                                    \
        ceqds3rr(W(XG), W(XG), W(XS))

#define ceqds_ld(XG, MS, DS)                                                \
        ceqds3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define ceqds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneds_rr(XG, XS)                                                    \
        cneds3rr(W(XG), W(XG), W(XS))

#define cneds_ld(XG, MS, DS)                                                \
        cneds3ld(W(XG), W(XG), W(MS), W(DS))

#define cneds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),RYG(XT), 0x03))                     \
        notdx_rx(W(XD))

#define cneds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xE8, RYG(XD),RYG(XS),   TmmM, 0x03))                     \
        notdx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltds_rr(XG, XS)                                                    \
        cltds3rr(W(XG), W(XG), W(XS))

#define cltds_ld(XG, MS, DS)                                                \
        cltds3ld(W(XG), W(XG), W(MS), W(DS))

#define cltds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XT),REG(XS), 0x03))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XT),RYG(XS), 0x03))

#define cltds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),TmmM,   REG(XS), 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),TmmM,   RYG(XS), 0x03))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleds_rr(XG, XS)                                                    \
        cleds3rr(W(XG), W(XG), W(XS))

#define cleds_ld(XG, MS, DS)                                                \
        cleds3ld(W(XG), W(XG), W(MS), W(DS))

#define cleds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XT),REG(XS), 0x03))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XT),RYG(XS), 0x03))

#define cleds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),TmmM,   REG(XS), 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),TmmM,   RYG(XS), 0x03))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtds_rr(XG, XS)                                                    \
        cgtds3rr(W(XG), W(XG), W(XS))

#define cgtds_ld(XG, MS, DS)                                                \
        cgtds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define cgtds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEB, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeds_rr(XG, XS)                                                    \
        cgeds3rr(W(XG), W(XG), W(XS))

#define cgeds_ld(XG, MS, DS)                                                \
        cgeds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeds3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define cgeds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xEA, RYG(XD),RYG(XS),   TmmM, 0x03))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_256  MN64_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_256  MF64_256   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN64_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0,  0, lb) ASM_END

#define SMF64_256(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP3(cgije, %%r0, -1, lb) ASM_END

#define mkjdx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMIT6(MXM(0x94, TmmM,   REG(XS),RYG(XS), 0x03))                     \
        EMIT6(MXM(0x94, TmmM,   TmmM,   TmmM,    0x02))                     \
        EMIT6(MXM(0x21, TMxx,   TmmM,   0x00,    0x03))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##64_256), EMPTY2)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x03))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x03))

#define cvzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x05, 0x03))

#define cvzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x05, 0x03))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x03))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x03))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x06, 0x03))

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x06, 0x03))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x03))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x03))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x07, 0x03))

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x07, 0x03))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x03))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x03))

#define cvnds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x04, 0x03))

#define cvnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x04, 0x03))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x03))

#define rndds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x03))

#define cvtds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC2, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC2, RYG(XD),RYG(XS),   0x00, 0x03))

#define cvtds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,      0x00, 0x03))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrds_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define cvrds_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC2, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndn_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x04, 0x03))

#define cvndn_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x04, 0x03))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdn_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC3, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC3, RYG(XD),RYG(XS),   0x00, 0x03))

#define cvtdn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC3, RYG(XD),TmmM,      0x00, 0x03))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndx_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x04, 0x03))

#define cvndx_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x04, 0x03))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdx_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC1, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC1, RYG(XD),RYG(XS),   0x00, 0x03))

#define cvtdx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC1, RYG(XD),TmmM,      0x00, 0x03))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x05, 0x03))

#define ruzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x05, 0x03))

#define cuzds_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x05, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x05, 0x03))

#define cuzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x05, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x05, 0x03))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x06, 0x03))

#define rupds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x06, 0x03))

#define cupds_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x06, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x06, 0x03))

#define cupds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x06, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x06, 0x03))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x07, 0x03))

#define rumds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x07, 0x03))

#define cumds_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x07, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x07, 0x03))

#define cumds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x07, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x07, 0x03))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x04, 0x03))

#define runds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x04, 0x03))

#define cunds_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x04, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x04, 0x03))

#define cunds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x04, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x04, 0x03))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),   0x00, 0x03))

#define rudds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC7, RYG(XD),TmmM,      0x00, 0x03))

#define cutds_rr(XD, XS)                                                    \
        EMIT6(MZM(0xC0, REG(XD),REG(XS),   0x00, 0x03))                     \
        EMIT6(MZM(0xC0, RYG(XD),RYG(XS),   0x00, 0x03))

#define cutds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,      0x00, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,      0x00, 0x03))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurds_rr(XD, XS, mode)                                              \
        EMIT6(MZM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MZM(0xC7, RYG(XD),RYG(XS),(RT_SIMD_MODE_##mode&3), 0x03))

#define curds_rr(XD, XS, mode)                                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, REG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))     \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VYL(DS), B2(DS), L2(DS)))  \
        EMIT6(MZM(0xC0, RYG(XD),TmmM,   (RT_SIMD_MODE_##mode&3), 0x03))

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define adddx_rr(XG, XS)                                                    \
        adddx3rr(W(XG), W(XG), W(XS))

#define adddx_ld(XG, MS, DS)                                                \
        adddx3ld(W(XG), W(XG), W(MS), W(DS))

#define adddx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define adddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF3, RYG(XD),RYG(XS),   TmmM, 0x03))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subdx_rr(XG, XS)                                                    \
        subdx3rr(W(XG), W(XG), W(XS))

#define subdx_ld(XG, MS, DS)                                                \
        subdx3ld(W(XG), W(XG), W(MS), W(DS))

#define subdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define subdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF7, RYG(XD),RYG(XS),   TmmM, 0x03))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define muldx_rr(XG, XS)                                                    \
        muldx3rr(W(XG), W(XG), W(XS))

#define muldx_ld(XG, MS, DS)                                                \
        muldx3ld(W(XG), W(XG), W(MS), W(DS))

#define muldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        muldx_rx(W(XD))

#define muldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        muldx_rx(W(XD))

#define muldx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

        /* div, rem are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shldx_ri(XG, IS)                                                    \
        shldx3ri(W(XG), W(XG), W(IS))

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx3ld(W(XG), W(XG), W(MS), W(DS))

#define shldx3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlzx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdx_ri(XG, IS)                                                    \
        shrdx3ri(W(XG), W(XG), W(IS))

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdx3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdn_ri(XG, IS)                                                    \
        shrdn3ri(W(XG), W(XG), W(IS))

#define shrdn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdn3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MT), W(DT))                                       \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx3rr(W(XG), W(XG), W(XS))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx3ld(W(XG), W(XG), W(MS), W(DS))

#define svldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svldx_rx(W(XD))

#define svldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svldx_rx(W(XD))

#define svldx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx3rr(W(XG), W(XG), W(XS))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrdx_rx(W(XD))

#define svrdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrdx_rx(W(XD))

#define svrdx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn3rr(W(XG), W(XG), W(XS))

#define svrdn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdn3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrdn_rx(W(XD))

#define svrdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrdn_rx(W(XD))

#define svrdn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mindx_rr(XG, XS)                                                    \
        mindx3rr(W(XG), W(XG), W(XS))

#define mindx_ld(XG, MS, DS)                                                \
        mindx3ld(W(XG), W(XG), W(MS), W(DS))

#define mindx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xFC, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define mindx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFC, RYG(XD),RYG(XS),   TmmM, 0x03))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mindn_rr(XG, XS)                                                    \
        mindn3rr(W(XG), W(XG), W(XS))

#define mindn_ld(XG, MS, DS)                                                \
        mindn3ld(W(XG), W(XG), W(MS), W(DS))

#define mindn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xFE, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define mindn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFE, RYG(XD),RYG(XS),   TmmM, 0x03))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxdx_rr(XG, XS)                                                    \
        maxdx3rr(W(XG), W(XG), W(XS))

#define maxdx_ld(XG, MS, DS)                                                \
        maxdx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xFD, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define maxdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFD, RYG(XD),RYG(XS),   TmmM, 0x03))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxdn_rr(XG, XS)                                                    \
        maxdn3rr(W(XG), W(XG), W(XS))

#define maxdn_ld(XG, MS, DS)                                                \
        maxdn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxdn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xFF, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define maxdn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFF, RYG(XD),RYG(XS),   TmmM, 0x03))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqdx_rr(XG, XS)                                                    \
        ceqdx3rr(W(XG), W(XG), W(XS))

#define ceqdx_ld(XG, MS, DS)                                                \
        ceqdx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define ceqdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnedx_rr(XG, XS)                                                    \
        cnedx3rr(W(XG), W(XG), W(XS))

#define cnedx_ld(XG, MS, DS)                                                \
        cnedx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnedx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),RYG(XT), 0x03))                     \
        notdx_rx(W(XD))

#define cnedx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF8, RYG(XD),RYG(XS),   TmmM, 0x03))                     \
        notdx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltdx_rr(XG, XS)                                                    \
        cltdx3rr(W(XG), W(XG), W(XS))

#define cltdx_ld(XG, MS, DS)                                                \
        cltdx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XT),REG(XS), 0x03))                     \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XT),RYG(XS), 0x03))

#define cltdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),TmmM,   REG(XS), 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, RYG(XD),TmmM,   RYG(XS), 0x03))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltdn_rr(XG, XS)                                                    \
        cltdn3rr(W(XG), W(XG), W(XS))

#define cltdn_ld(XG, MS, DS)                                                \
        cltdn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltdn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XT),REG(XS), 0x03))                     \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XT),RYG(XS), 0x03))

#define cltdn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),TmmM,   REG(XS), 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, RYG(XD),TmmM,   RYG(XS), 0x03))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cledx_rr(XG, XS)                                                    \
        cledx3rr(W(XG), W(XG), W(XS))

#define cledx_ld(XG, MS, DS)                                                \
        cledx3ld(W(XG), W(XG), W(MS), W(DS))

#define cledx3rr(XD, XS, XT)                                                \
        cgtdx3rr(W(XD), W(XS), W(XT))                                       \
        notdx_rx(W(XD))

#define cledx3ld(XD, XS, MT, DT)                                            \
        cgtdx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notdx_rx(W(XD))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cledn_rr(XG, XS)                                                    \
        cledn3rr(W(XG), W(XG), W(XS))

#define cledn_ld(XG, MS, DS)                                                \
        cledn3ld(W(XG), W(XG), W(MS), W(DS))

#define cledn3rr(XD, XS, XT)                                                \
        cgtdn3rr(W(XD), W(XS), W(XT))                                       \
        notdx_rx(W(XD))

#define cledn3ld(XD, XS, MT, DT)                                            \
        cgtdn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notdx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtdx_rr(XG, XS)                                                    \
        cgtdx3rr(W(XG), W(XG), W(XS))

#define cgtdx_ld(XG, MS, DS)                                                \
        cgtdx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtdx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define cgtdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xF9, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtdn_rr(XG, XS)                                                    \
        cgtdn3rr(W(XG), W(XG), W(XS))

#define cgtdn_ld(XG, MS, DS)                                                \
        cgtdn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtdn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),REG(XT), 0x03))                     \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XS),RYG(XT), 0x03))

#define cgtdn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),   TmmM, 0x03))                     \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VYL(DT), B2(DT), L2(DT)))  \
        EMIT6(MXM(0xFB, RYG(XD),RYG(XS),   TmmM, 0x03))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgedx_rr(XG, XS)                                                    \
        cgedx3rr(W(XG), W(XG), W(XS))

#define cgedx_ld(XG, MS, DS)                                                \
        cgedx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgedx3rr(XD, XS, XT)                                                \
        cltdx3rr(W(XD), W(XS), W(XT))                                       \
        notdx_rx(W(XD))

#define cgedx3ld(XD, XS, MT, DT)                                            \
        cltdx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notdx_rx(W(XD))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgedn_rr(XG, XS)                                                    \
        cgedn3rr(W(XG), W(XG), W(XS))

#define cgedn_ld(XG, MS, DS)                                                \
        cgedn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgedn3rr(XD, XS, XT)                                                \
        cltdn3rr(W(XD), W(XS), W(XT))                                       \
        notdx_rx(W(XD))

#define cgedn3ld(XD, XS, MT, DT)                                            \
        cltdn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notdx_rx(W(XD))

#endif /* (defined RT_Z64) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_Z64_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ZQF_128X1V1_H
#define RT_RTARCH_ZQF_128X1V1_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_zQF_128x*v1.h: Implementation of s390x SIMD instructions.
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

/****************   scalar quad-precision generic move/logic   ****************/

/* mov (D = S) */

#define movvx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x56, REG(XD),REG(XS),   0x00, 0x00))

#define movvx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, REG(XD),MOD(MS),REG(MS), VAL(DS), B2(DS), P2(DS)))

#define movvx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMIT6(MPM(0x0E, REG(XS),MOD(MD),REG(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvvx_rr(XG, XS)                                                    \
        andvx_rr(W(XS), Xmm0)                                               \
        annvx_rr(Xmm0, W(XG))                                               \
        orrvx_rr(Xmm0, W(XS))                                               \
        movvx_rr(W(XG), Xmm0)

#define mmvvx_ld(XG, MS, DS)                                                \
        notvx_rx(Xmm0)                                                      \
        andvx_rr(W(XG), Xmm0)                                               \
        annvx_ld(Xmm0, W(MS), W(DS))                                        \
        orrvx_rr(W(XG), Xmm0)

#define mmvvx_st(XS, MG, DG)                                                \
        andvx_rr(W(XS), Xmm0)                                               \
        annvx_ld(Xmm0, W(MG), W(DG))                                        \
        orrvx_rr(Xmm0, W(XS))                                               \
        movvx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andvx_rr(XG, XS)                                                    \
        andvx3rr(W(XG), W(XG), W(XS))

#define andvx_ld(XG, MS, DS)                                                \
        andvx3ld(W(XG), W(XG), W(MS), W(DS))

#define andvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x68, REG(XD),REG(XS),REG(XT), 0x00))

#define andvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x68, REG(XD),REG(XS),   TmmM, 0x00))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annvx_rr(XG, XS)                                                    \
        annvx3rr(W(XG), W(XG), W(XS))

#define annvx_ld(XG, MS, DS)                                                \
        annvx3ld(W(XG), W(XG), W(MS), W(DS))

#define annvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x69, REG(XD),REG(XT),REG(XS), 0x00))

#define annvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x69, REG(XD),TmmM,   REG(XS), 0x00))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrvx_rr(XG, XS)                                                    \
        orrvx3rr(W(XG), W(XG), W(XS))

#define orrvx_ld(XG, MS, DS)                                                \
        orrvx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),REG(XT), 0x00))

#define orrvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6A, REG(XD),REG(XS),   TmmM, 0x00))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornvx_rr(XG, XS)                                                    \
        ornvx3rr(W(XG), W(XG), W(XS))

#define ornvx_ld(XG, MS, DS)                                                \
        ornvx3ld(W(XG), W(XG), W(MS), W(DS))

#define ornvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6F, REG(XD),REG(XT),REG(XS), 0x00))

#define ornvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6F, REG(XD),TmmM,   REG(XS), 0x00))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorvx_rr(XG, XS)                                                    \
        xorvx3rr(W(XG), W(XG), W(XS))

#define xorvx_ld(XG, MS, DS)                                                \
        xorvx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),REG(XT), 0x00))

#define xorvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0x6D, REG(XD),REG(XS),   TmmM, 0x00))

/* not (G = ~G), (D = ~S) */

#define notvx_rx(XG)                                                        \
        notvx_rr(W(XG), W(XG))

#define notvx_rr(XD, XS)                                                    \
        EMIT6(MXM(0x6B, REG(XD),REG(XS),REG(XS), 0x00))

/*************   scalar quad-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negvs_rx(XG)                                                        \
        negvs_rr(W(XG), W(XG))

#define negvs_rr(XD, XS)                                                    \
        EMIT6(MYM(0xCC, REG(XD),REG(XS),   0x00, 0x04))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addvs_rr(XG, XS)                                                    \
        addvs3rr(W(XG), W(XG), W(XS))

#define addvs_ld(XG, MS, DS)                                                \
        addvs3ld(W(XG), W(XG), W(MS), W(DS))

#define addvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),REG(XT), 0x04))

#define addvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE3, REG(XD),REG(XS),   TmmM, 0x04))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subvs_rr(XG, XS)                                                    \
        subvs3rr(W(XG), W(XG), W(XS))

#define subvs_ld(XG, MS, DS)                                                \
        subvs3ld(W(XG), W(XG), W(MS), W(DS))

#define subvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),REG(XT), 0x04))

#define subvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE2, REG(XD),REG(XS),   TmmM, 0x04))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulvs_rr(XG, XS)                                                    \
        mulvs3rr(W(XG), W(XG), W(XS))

#define mulvs_ld(XG, MS, DS)                                                \
        mulvs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),REG(XT), 0x04))

#define mulvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE7, REG(XD),REG(XS),   TmmM, 0x04))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divvs_rr(XG, XS)                                                    \
        divvs3rr(W(XG), W(XG), W(XS))

#define divvs_ld(XG, MS, DS)                                                \
        divvs3ld(W(XG), W(XG), W(MS), W(DS))

#define divvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),REG(XT), 0x04))

#define divvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE5, REG(XD),REG(XS),   TmmM, 0x04))

/* sqr (D = sqrt S) */

#define sqrvs_rr(XD, XS)                                                    \
        EMIT6(MWM(0xCE, REG(XD),REG(XS),   0x00, 0x04))

#define sqrvs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MWM(0xCE, REG(XD),TmmM,      0x00, 0x04))

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

#define fmavs_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),REG(XT), 0x04))

#define fmavs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MUM(0x8F, REG(XG),REG(XS),   TmmM, 0x04))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsvs_rr(XG, XS, XT)                                                \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),REG(XT), 0x04))

#define fmsvs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MUM(0x9E, REG(XG),REG(XS),   TmmM, 0x04))

#endif /* RT_SIMD_COMPAT_FMS */

/**************   scalar quad-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minvs_rr(XG, XS)                                                    \
        minvs3rr(W(XG), W(XG), W(XS))

#define minvs_ld(XG, MS, DS)                                                \
        minvs3ld(W(XG), W(XG), W(MS), W(DS))

#define minvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),REG(XT), 0x04))

#define minvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEE, REG(XD),REG(XS),   TmmM, 0x04))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxvs_rr(XG, XS)                                                    \
        maxvs3rr(W(XG), W(XG), W(XS))

#define maxvs_ld(XG, MS, DS)                                                \
        maxvs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),REG(XT), 0x04))

#define maxvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEF, REG(XD),REG(XS),   TmmM, 0x04))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqvs_rr(XG, XS)                                                    \
        ceqvs3rr(W(XG), W(XG), W(XS))

#define ceqvs_ld(XG, MS, DS)                                                \
        ceqvs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x04))

#define ceqvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x04))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnevs_rr(XG, XS)                                                    \
        cnevs3rr(W(XG), W(XG), W(XS))

#define cnevs_ld(XG, MS, DS)                                                \
        cnevs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnevs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),REG(XT), 0x04))                     \
        notvx_rx(W(XD))

#define cnevs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xE8, REG(XD),REG(XS),   TmmM, 0x04))                     \
        notvx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltvs_rr(XG, XS)                                                    \
        cltvs3rr(W(XG), W(XG), W(XS))

#define cltvs_ld(XG, MS, DS)                                                \
        cltvs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XT),REG(XS), 0x04))

#define cltvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),TmmM,   REG(XS), 0x04))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clevs_rr(XG, XS)                                                    \
        clevs3rr(W(XG), W(XG), W(XS))

#define clevs_ld(XG, MS, DS)                                                \
        clevs3ld(W(XG), W(XG), W(MS), W(DS))

#define clevs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XT),REG(XS), 0x04))

#define clevs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),TmmM,   REG(XS), 0x04))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtvs_rr(XG, XS)                                                    \
        cgtvs3rr(W(XG), W(XG), W(XS))

#define cgtvs_ld(XG, MS, DS)                                                \
        cgtvs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtvs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),REG(XT), 0x04))

#define cgtvs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEB, REG(XD),REG(XS),   TmmM, 0x04))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgevs_rr(XG, XS)                                                    \
        cgevs3rr(W(XG), W(XG), W(XS))

#define cgevs_ld(XG, MS, DS)                                                \
        cgevs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgevs3rr(XD, XS, XT)                                                \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),REG(XT), 0x04))

#define cgevs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MWM(0xEA, REG(XD),REG(XS),   TmmM, 0x04))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE00_128  MN00_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL00_128  MF00_128   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN00_128(xs, lb) /* not portable, do not use outside */            \
        EMIT6(MXM(0xD8, xs,TmmQ,   0x00, 0x00))                             \
        ASM_BEG ASM_OP1(je, lb) ASM_END

#define SMF00_128(xs, lb) /* not portable, do not use outside */            \
        EMIT6(MXM(0xD8, xs,TmmQ,   0x00, 0x00))                             \
        ASM_BEG ASM_OP1(jo, lb) ASM_END

#define mkjvx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##00_128), EMPTY2)

/**************   scalar quad-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzvs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x04))

#define rnzvs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x04))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpvs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x04))

#define rnpvs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x04))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmvs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x04))

#define rnmvs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x04))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnvs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x04))

#define rnnvs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x04))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndvs_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x04))

#define rndvs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x04))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrvs_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x04))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzvs_rr(XD, XS)     /* round towards zero */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x05, 0x04))

#define ruzvs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x05, 0x04))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupvs_rr(XD, XS)     /* round towards +inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x06, 0x04))

#define rupvs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x06, 0x04))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumvs_rr(XD, XS)     /* round towards -inf */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x07, 0x04))

#define rumvs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x07, 0x04))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runvs_rr(XD, XS)     /* round towards near */                       \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x04, 0x04))

#define runvs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x04, 0x04))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudvs_rr(XD, XS)                                                    \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),   0x00, 0x04))

#define rudvs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MS),  REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMIT6(MYM(0xC7, REG(XD),TmmM,      0x00, 0x04))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurvs_rr(XD, XS, mode)                                              \
        EMIT6(MYM(0xC7, REG(XD),REG(XS),(RT_SIMD_MODE_##mode&3), 0x04))

/*************   scalar quad-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addvx_rr(XG, XS)                                                    \
        addvx3rr(W(XG), W(XG), W(XS))

#define addvx_ld(XG, MS, DS)                                                \
        addvx3ld(W(XG), W(XG), W(MS), W(DS))

#define addvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),REG(XT), 0x04))

#define addvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF3, REG(XD),REG(XS),   TmmM, 0x04))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subvx_rr(XG, XS)                                                    \
        subvx3rr(W(XG), W(XG), W(XS))

#define subvx_ld(XG, MS, DS)                                                \
        subvx3ld(W(XG), W(XG), W(MS), W(DS))

#define subvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),REG(XT), 0x04))

#define subvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF7, REG(XD),REG(XS),   TmmM, 0x04))

#if RT_Z64 >= 17 /* only on z17 and beyond */

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulvx_rr(XG, XS)                                                    \
        mulvx3rr(W(XG), W(XG), W(XS))

#define mulvx_ld(XG, MS, DS)                                                \
        mulvx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),REG(XT), 0x04))

#define mulvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xA2, REG(XD),REG(XS),   TmmM, 0x04))

/*****************   scalar quad-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minvx_rr(XG, XS)                                                    \
        minvx3rr(W(XG), W(XG), W(XS))

#define minvx_ld(XG, MS, DS)                                                \
        minvx3ld(W(XG), W(XG), W(MS), W(DS))

#define minvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),REG(XT), 0x04))

#define minvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xFC, REG(XD),REG(XS),   TmmM, 0x04))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minvn_rr(XG, XS)                                                    \
        minvn3rr(W(XG), W(XG), W(XS))

#define minvn_ld(XG, MS, DS)                                                \
        minvn3ld(W(XG), W(XG), W(MS), W(DS))

#define minvn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),REG(XT), 0x04))

#define minvn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xFE, REG(XD),REG(XS),   TmmM, 0x04))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxvx_rr(XG, XS)                                                    \
        maxvx3rr(W(XG), W(XG), W(XS))

#define maxvx_ld(XG, MS, DS)                                                \
        maxvx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),REG(XT), 0x04))

#define maxvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xFD, REG(XD),REG(XS),   TmmM, 0x04))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxvn_rr(XG, XS)                                                    \
        maxvn3rr(W(XG), W(XG), W(XS))

#define maxvn_ld(XG, MS, DS)                                                \
        maxvn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxvn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),REG(XT), 0x04))

#define maxvn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xFF, REG(XD),REG(XS),   TmmM, 0x04))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqvx_rr(XG, XS)                                                    \
        ceqvx3rr(W(XG), W(XG), W(XS))

#define ceqvx_ld(XG, MS, DS)                                                \
        ceqvx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x04))

#define ceqvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x04))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnevx_rr(XG, XS)                                                    \
        cnevx3rr(W(XG), W(XG), W(XS))

#define cnevx_ld(XG, MS, DS)                                                \
        cnevx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnevx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),REG(XT), 0x04))                     \
        notvx_rx(W(XD))

#define cnevx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF8, REG(XD),REG(XS),   TmmM, 0x04))                     \
        notvx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltvx_rr(XG, XS)                                                    \
        cltvx3rr(W(XG), W(XG), W(XS))

#define cltvx_ld(XG, MS, DS)                                                \
        cltvx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XT),REG(XS), 0x04))

#define cltvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),TmmM,   REG(XS), 0x04))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltvn_rr(XG, XS)                                                    \
        cltvn3rr(W(XG), W(XG), W(XS))

#define cltvn_ld(XG, MS, DS)                                                \
        cltvn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltvn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XT),REG(XS), 0x04))

#define cltvn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),TmmM,   REG(XS), 0x04))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clevx_rr(XG, XS)                                                    \
        clevx3rr(W(XG), W(XG), W(XS))

#define clevx_ld(XG, MS, DS)                                                \
        clevx3ld(W(XG), W(XG), W(MS), W(DS))

#define clevx3rr(XD, XS, XT)                                                \
        cgtvx3rr(W(XD), W(XS), W(XT))                                       \
        notvx_rx(W(XD))

#define clevx3ld(XD, XS, MT, DT)                                            \
        cgtvx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notvx_rx(W(XD))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clevn_rr(XG, XS)                                                    \
        clevn3rr(W(XG), W(XG), W(XS))

#define clevn_ld(XG, MS, DS)                                                \
        clevn3ld(W(XG), W(XG), W(MS), W(DS))

#define clevn3rr(XD, XS, XT)                                                \
        cgtvn3rr(W(XD), W(XS), W(XT))                                       \
        notvx_rx(W(XD))

#define clevn3ld(XD, XS, MT, DT)                                            \
        cgtvn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notvx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtvx_rr(XG, XS)                                                    \
        cgtvx3rr(W(XG), W(XG), W(XS))

#define cgtvx_ld(XG, MS, DS)                                                \
        cgtvx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtvx3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),REG(XT), 0x04))

#define cgtvx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xF9, REG(XD),REG(XS),   TmmM, 0x04))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtvn_rr(XG, XS)                                                    \
        cgtvn3rr(W(XG), W(XG), W(XS))

#define cgtvn_ld(XG, MS, DS)                                                \
        cgtvn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtvn3rr(XD, XS, XT)                                                \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),REG(XT), 0x04))

#define cgtvn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMIT6(MPM(0x06, TmmM, MOD(MT),  REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMIT6(MXM(0xFB, REG(XD),REG(XS),   TmmM, 0x04))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgevx_rr(XG, XS)                                                    \
        cgevx3rr(W(XG), W(XG), W(XS))

#define cgevx_ld(XG, MS, DS)                                                \
        cgevx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgevx3rr(XD, XS, XT)                                                \
        cltvx3rr(W(XD), W(XS), W(XT))                                       \
        notvx_rx(W(XD))

#define cgevx3ld(XD, XS, MT, DT)                                            \
        cltvx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notvx_rx(W(XD))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgevn_rr(XG, XS)                                                    \
        cgevn3rr(W(XG), W(XG), W(XS))

#define cgevn_ld(XG, MS, DS)                                                \
        cgevn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgevn3rr(XD, XS, XT)                                                \
        cltvn3rr(W(XD), W(XS), W(XT))                                       \
        notvx_rx(W(XD))

#define cgevn3ld(XD, XS, MT, DT)                                            \
        cltvn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notvx_rx(W(XD))

#endif /* RT_Z64 >= 17 */

#endif /* (defined RT_Z64) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_ZQF_128X1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

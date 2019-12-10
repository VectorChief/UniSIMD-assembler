/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_128X1V1_H
#define RT_RTARCH_A64_128X1V1_H

#include "rtarch_a32_128x1v1.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64_128x1v1.h: Implementation of AArch64 fp64 NEON instructions.
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
 * cmdi*_** - applies [cmd] to 32-bit SIMD element args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit SIMD element args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size SIMD element args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit SIMD element args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit SIMD element args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size SIMD element args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD element args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size SIMD element args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit SIMD element args, packed-var-len
 *
 * cmd*x_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
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
 * should propagate QNaNs unchanged, however this behavior hasn't been verified.
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

#if (RT_128X1 != 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmjx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movts_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movjx_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))

#define movjx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movjx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3C800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvjx_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))

#define mmvjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))

#define mmvjx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3C800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andjx_rr(XG, XS)                                                    \
        andjx3rr(W(XG), W(XG), W(XS))

#define andjx_ld(XG, MS, DS)                                                \
        andjx3ld(W(XG), W(XG), W(MS), W(DS))

#define andjx3rr(XD, XS, XT)                                                \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define andjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annjx_rr(XG, XS)                                                    \
        annjx3rr(W(XG), W(XG), W(XS))

#define annjx_ld(XG, MS, DS)                                                \
        annjx3ld(W(XG), W(XG), W(MS), W(DS))

#define annjx3rr(XD, XS, XT)                                                \
        EMITW(0x4E601C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define annjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E601C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrjx_rr(XG, XS)                                                    \
        orrjx3rr(W(XG), W(XG), W(XS))

#define orrjx_ld(XG, MS, DS)                                                \
        orrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrjx3rr(XD, XS, XT)                                                \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornjx_rr(XG, XS)                                                    \
        ornjx3rr(W(XG), W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        ornjx3ld(W(XG), W(XG), W(MS), W(DS))

#define ornjx3rr(XD, XS, XT)                                                \
        EMITW(0x4EE01C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define ornjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE01C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorjx_rr(XG, XS)                                                    \
        xorjx3rr(W(XG), W(XG), W(XS))

#define xorjx_ld(XG, MS, DS)                                                \
        xorjx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorjx3rr(XD, XS, XT)                                                \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notjx_rx(XG)                                                        \
        notjx_rr(W(XG), W(XG))

#define notjx_rr(XD, XS)                                                    \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XS), 0x00))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negjs_rx(XG)                                                        \
        negjs_rr(W(XG), W(XG))

#define negjs_rr(XD, XS)                                                    \
        EMITW(0x6EE0F800 | MXM(REG(XD), REG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addjs_rr(XG, XS)                                                    \
        addjs3rr(W(XG), W(XG), W(XS))

#define addjs_ld(XG, MS, DS)                                                \
        addjs3ld(W(XG), W(XG), W(MS), W(DS))

#define addjs3rr(XD, XS, XT)                                                \
        EMITW(0x4E60D400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60D400 | MXM(REG(XD), REG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  adpjs3rr
#define adpjs3rr(XD, XS, XT)                                                \
        EMITW(0x6E60D400 | MXM(REG(XD), REG(XS), REG(XT)))

#undef  adpjs3ld
#define adpjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60D400 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subjs_rr(XG, XS)                                                    \
        subjs3rr(W(XG), W(XG), W(XS))

#define subjs_ld(XG, MS, DS)                                                \
        subjs3ld(W(XG), W(XG), W(MS), W(DS))

#define subjs3rr(XD, XS, XT)                                                \
        EMITW(0x4EE0D400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0D400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define muljs_rr(XG, XS)                                                    \
        muljs3rr(W(XG), W(XG), W(XS))

#define muljs_ld(XG, MS, DS)                                                \
        muljs3ld(W(XG), W(XG), W(MS), W(DS))

#define muljs3rr(XD, XS, XT)                                                \
        EMITW(0x6E60DC00 | MXM(REG(XD), REG(XS), REG(XT)))

#define muljs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60DC00 | MXM(REG(XD), REG(XS), TmmM))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divjs_rr(XG, XS)                                                    \
        divjs3rr(W(XG), W(XG), W(XS))

#define divjs_ld(XG, MS, DS)                                                \
        divjs3ld(W(XG), W(XG), W(MS), W(DS))

#define divjs3rr(XD, XS, XT)                                                \
        EMITW(0x6E60FC00 | MXM(REG(XD), REG(XS), REG(XT)))

#define divjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60FC00 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrjs_rr(XD, XS)                                                    \
        EMITW(0x6EE1F800 | MXM(REG(XD), REG(XS), 0x00))

#define sqrjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE1F800 | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcejs_rr(XD, XS)                                                    \
        EMITW(0x4EE1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsjs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E60FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsejs_rr(XD, XS)                                                    \
        EMITW(0x6EE1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssjs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E60DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EE0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmajs_rr(XG, XS, XT)                                                \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsjs_rr(XG, XS, XT)                                                \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minjs_rr(XG, XS)                                                    \
        minjs3rr(W(XG), W(XG), W(XS))

#define minjs_ld(XG, MS, DS)                                                \
        minjs3ld(W(XG), W(XG), W(MS), W(DS))

#define minjs3rr(XD, XS, XT)                                                \
        EMITW(0x4EE0F400 | MXM(REG(XD), REG(XS), REG(XT)))

#define minjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0F400 | MXM(REG(XD), REG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxjs_rr(XG, XS)                                                    \
        maxjs3rr(W(XG), W(XG), W(XS))

#define maxjs_ld(XG, MS, DS)                                                \
        maxjs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxjs3rr(XD, XS, XT)                                                \
        EMITW(0x4E60F400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60F400 | MXM(REG(XD), REG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqjs_rr(XG, XS)                                                    \
        ceqjs3rr(W(XG), W(XG), W(XS))

#define ceqjs_ld(XG, MS, DS)                                                \
        ceqjs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqjs3rr(XD, XS, XT)                                                \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnejs_rr(XG, XS)                                                    \
        cnejs3rr(W(XG), W(XG), W(XS))

#define cnejs_ld(XG, MS, DS)                                                \
        cnejs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnejs3rr(XD, XS, XT)                                                \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cnejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltjs_rr(XG, XS)                                                    \
        cltjs3rr(W(XG), W(XG), W(XS))

#define cltjs_ld(XG, MS, DS)                                                \
        cltjs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltjs3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clejs_rr(XG, XS)                                                    \
        clejs3rr(W(XG), W(XG), W(XS))

#define clejs_ld(XG, MS, DS)                                                \
        clejs3ld(W(XG), W(XG), W(MS), W(DS))

#define clejs3rr(XD, XS, XT)                                                \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define clejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtjs_rr(XG, XS)                                                    \
        cgtjs3rr(W(XG), W(XG), W(XS))

#define cgtjs_ld(XG, MS, DS)                                                \
        cgtjs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtjs3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtjs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgejs_rr(XG, XS)                                                    \
        cgejs3rr(W(XG), W(XG), W(XS))

#define cgejs_ld(XG, MS, DS)                                                \
        cgejs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgejs3rr(XD, XS, XT)                                                \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgejs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_128     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_128     0x04    /*  all satisfy the condition */

#define mkjjx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4EB1B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask##64_128))                     \
        jezxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE19800 | MXM(REG(XD), REG(XS), 0x00))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE19800 | MXM(REG(XD), TmmM,    0x00))

#define cvzjs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE1B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1B800 | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpjs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE18800 | MXM(REG(XD), REG(XS), 0x00))

#define rnpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE18800 | MXM(REG(XD), TmmM,    0x00))

#define cvpjs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE1A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1A800 | MXM(REG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmjs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E619800 | MXM(REG(XD), REG(XS), 0x00))

#define rnmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E619800 | MXM(REG(XD), TmmM,    0x00))

#define cvmjs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E61B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61B800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E618800 | MXM(REG(XD), TmmM,    0x00))

#define cvnjs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvnjs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61A800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnjn_rr(XD, XS)     /* round towards near */                       \
        cvtjn_rr(W(XD), W(XS))

#define cvnjn_ld(XD, MS, DS) /* round towards near */                       \
        cvtjn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndjs_rr(XD, XS)                                                    \
        EMITW(0x6EE19800 | MXM(REG(XD), REG(XS), 0x00))

#define rndjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE19800 | MXM(REG(XD), TmmM,    0x00))

#define cvtjs_rr(XD, XS)                                                    \
        rndjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvtjs_ld(XD, MS, DS)                                                \
        rndjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtjn_rr(XD, XS)                                                    \
        EMITW(0x4E61D800 | MXM(REG(XD), REG(XS), 0x00))

#define cvtjn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61D800 | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrjs_rr(XD, XS, mode)                                              \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvrjs_rr(XD, XS, mode)                                              \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addjx_rr(XG, XS)                                                    \
        addjx3rr(W(XG), W(XG), W(XS))

#define addjx_ld(XG, MS, DS)                                                \
        addjx3ld(W(XG), W(XG), W(MS), W(DS))

#define addjx3rr(XD, XS, XT)                                                \
        EMITW(0x4EE08400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE08400 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subjx_rr(XG, XS)                                                    \
        subjx3rr(W(XG), W(XG), W(XS))

#define subjx_ld(XG, MS, DS)                                                \
        subjx3ld(W(XG), W(XG), W(MS), W(DS))

#define subjx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE08400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE08400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define muljx_rr(XG, XS)                                                    \
        muljx3rr(W(XG), W(XG), W(XS))

#define muljx_ld(XG, MS, DS)                                                \
        muljx3ld(W(XG), W(XG), W(MS), W(DS))

#define muljx3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define muljx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shljx_ri(XG, IS)                                                    \
        shljx3ri(W(XG), W(XG), W(IS))

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shljx3ld(W(XG), W(XG), W(MS), W(DS))

#define shljx3ri(XD, XS, IT)                                                \
        EMITW(0x4F400400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 64) & 0x00005000) | (M(VAL(IT) > 63) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F & VAL(IT)) << 16)

#define shljx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrjx_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shrjx3ri(W(XG), W(XG), W(IS))

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrjx3ri(XD, XS, IT)                                                \
        EMITW(0x4F400400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
                                                 (0x3F &-VAL(IT)) << 16)

#define shrjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrjn_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shrjn3ri(W(XG), W(XG), W(IS))

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrjn3ri(XD, XS, IT)                                                \
        EMITW(0x4F400400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
                                                 (0x3F &-VAL(IT)) << 16)

#define shrjn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx3rr(W(XG), W(XG), W(XS))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx3ld(W(XG), W(XG), W(MS), W(DS))

#define svljx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), REG(XT)))

#define svljx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx3rr(W(XG), W(XG), W(XS))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))

#define svrjx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn3rr(W(XG), W(XG), W(XS))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjn3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XD), REG(XS), TmmM))

#define svrjn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*********   scalar double-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movts_rr(XD, XS)                                                    \
        EMITW(0x5E080400 | MXM(REG(XD), REG(XS), 0x00))

#define movts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xFC400000 | MPM(REG(XD), MOD(MS), VXL(DS), B1(DS), P1(DS)))

#define movts_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xFC000000 | MPM(REG(XS), MOD(MD), VXL(DD), B1(DD), P1(DD)))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addts_rr(XG, XS)                                                    \
        addts3rr(W(XG), W(XG), W(XS))

#define addts_ld(XG, MS, DS)                                                \
        addts3ld(W(XG), W(XG), W(MS), W(DS))

#define addts3rr(XD, XS, XT)                                                \
        EMITW(0x1E602800 | MXM(REG(XD), REG(XS), REG(XT)))

#define addts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E602800 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subts_rr(XG, XS)                                                    \
        subts3rr(W(XG), W(XG), W(XS))

#define subts_ld(XG, MS, DS)                                                \
        subts3ld(W(XG), W(XG), W(MS), W(DS))

#define subts3rr(XD, XS, XT)                                                \
        EMITW(0x1E603800 | MXM(REG(XD), REG(XS), REG(XT)))

#define subts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E603800 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mults_rr(XG, XS)                                                    \
        mults3rr(W(XG), W(XG), W(XS))

#define mults_ld(XG, MS, DS)                                                \
        mults3ld(W(XG), W(XG), W(MS), W(DS))

#define mults3rr(XD, XS, XT)                                                \
        EMITW(0x1E600800 | MXM(REG(XD), REG(XS), REG(XT)))

#define mults3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E600800 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divts_rr(XG, XS)                                                    \
        divts3rr(W(XG), W(XG), W(XS))

#define divts_ld(XG, MS, DS)                                                \
        divts3ld(W(XG), W(XG), W(MS), W(DS))

#define divts3rr(XD, XS, XT)                                                \
        EMITW(0x1E601800 | MXM(REG(XD), REG(XS), REG(XT)))

#define divts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E601800 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrts_rr(XD, XS)                                                    \
        EMITW(0x1E61C000 | MXM(REG(XD), REG(XS), 0x00))

#define sqrts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1E61C000 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcets_rr(XD, XS)                                                    \
        EMITW(0x5EE1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsts_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x5E60FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1E600800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsets_rr(XD, XS)                                                    \
        EMITW(0x7EE1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssts_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1E600800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x5EE0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1E600800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMITW(0x1F400000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1F400000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMITW(0x1F408000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1F408000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   scalar double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define mints_rr(XG, XS)                                                    \
        mints3rr(W(XG), W(XG), W(XS))

#define mints_ld(XG, MS, DS)                                                \
        mints3ld(W(XG), W(XG), W(MS), W(DS))

#define mints3rr(XD, XS, XT)                                                \
        EMITW(0x1E605800 | MXM(REG(XD), REG(XS), REG(XT)))

#define mints3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E605800 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxts_rr(XG, XS)                                                    \
        maxts3rr(W(XG), W(XG), W(XS))

#define maxts_ld(XG, MS, DS)                                                \
        maxts3ld(W(XG), W(XG), W(MS), W(DS))

#define maxts3rr(XD, XS, XT)                                                \
        EMITW(0x1E604800 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E604800 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqts_rr(XG, XS)                                                    \
        ceqts3rr(W(XG), W(XG), W(XS))

#define ceqts_ld(XG, MS, DS)                                                \
        ceqts3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqts3rr(XD, XS, XT)                                                \
        EMITW(0x5E60E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x5E60E400 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnets_rr(XG, XS)                                                    \
        cnets3rr(W(XG), W(XG), W(XS))

#define cnets_ld(XG, MS, DS)                                                \
        cnets3ld(W(XG), W(XG), W(MS), W(DS))

#define cnets3rr(XD, XS, XT)                                                \
        EMITW(0x5E60E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cnets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x5E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltts_rr(XG, XS)                                                    \
        cltts3rr(W(XG), W(XG), W(XS))

#define cltts_ld(XG, MS, DS)                                                \
        cltts3ld(W(XG), W(XG), W(MS), W(DS))

#define cltts3rr(XD, XS, XT)                                                \
        EMITW(0x7EE0E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7EE0E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clets_rr(XG, XS)                                                    \
        clets3rr(W(XG), W(XG), W(XS))

#define clets_ld(XG, MS, DS)                                                \
        clets3ld(W(XG), W(XG), W(MS), W(DS))

#define clets3rr(XD, XS, XT)                                                \
        EMITW(0x7E60E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define clets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7E60E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtts_rr(XG, XS)                                                    \
        cgtts3rr(W(XG), W(XG), W(XS))

#define cgtts_ld(XG, MS, DS)                                                \
        cgtts3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtts3rr(XD, XS, XT)                                                \
        EMITW(0x7EE0E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtts3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7EE0E400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgets_rr(XG, XS)                                                    \
        cgets3rr(W(XG), W(XG), W(XS))

#define cgets_ld(XG, MS, DS)                                                \
        cgets3ld(W(XG), W(XG), W(MS), W(DS))

#define cgets3rr(XD, XS, XT)                                                \
        EMITW(0x7E60E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgets3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xFC400000 | MPM(TmmM,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7E60E400 | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A64_128X1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_256X1V8_H
#define RT_RTARCH_X32_256X1V8_H

#include "rtarch_x64.h"

#define RT_SIMD_REGS_256        32

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_256x1v8.h: Implementation of x86_64 fp32 AVX512F/VL instructions.
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

#if (RT_256X1 == 8)

#ifndef RT_RTARCH_X64_128X1V2_H
#undef  RT_128X1
#define RT_128X1  2
#include "rtarch_x64_128x1v2.h"
#endif /* RT_RTARCH_X64_128X1V2_H */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

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
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movcx_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movcx_st(XS, MD, DD)                                                \
    ADR EVX(RXB(XS), RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvcx_rr(XG, XS)                                                    \
        ck1cx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(RXB(XG), RXB(XS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvcx_ld(XG, MS, DS)                                                \
        ck1cx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(RXB(XG), RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvcx_st(XS, MG, DG)                                                \
        ck1cx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(RXB(XS), RXB(MG),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define ck1cx_rm(XS, MT, DT) /* not portable, do not use outside */         \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 1, 1) EMITB(0x76)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andcx_rr(XG, XS)                                                    \
        andcx3rr(W(XG), W(XG), W(XS))

#define andcx_ld(XG, MS, DS)                                                \
        andcx3ld(W(XG), W(XG), W(MS), W(DS))

#define andcx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define anncx_rr(XG, XS)                                                    \
        anncx3rr(W(XG), W(XG), W(XS))

#define anncx_ld(XG, MS, DS)                                                \
        anncx3ld(W(XG), W(XG), W(MS), W(DS))

#define anncx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anncx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrcx_rr(XG, XS)                                                    \
        orrcx3rr(W(XG), W(XG), W(XS))

#define orrcx_ld(XG, MS, DS)                                                \
        orrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrcx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define orncx_rr(XG, XS)                                                    \
        notcx_rx(W(XG))                                                     \
        orrcx_rr(W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        notcx_rx(W(XG))                                                     \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orncx3rr(XD, XS, XT)                                                \
        notcx_rr(W(XD), W(XS))                                              \
        orrcx_rr(W(XD), W(XT))

#define orncx3ld(XD, XS, MT, DT)                                            \
        notcx_rr(W(XD), W(XS))                                              \
        orrcx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorcx_rr(XG, XS)                                                    \
        xorcx3rr(W(XG), W(XG), W(XS))

#define xorcx_ld(XG, MS, DS)                                                \
        xorcx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorcx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notcx_rx(XG)                                                        \
        notcx_rr(W(XG), W(XG))

#define notcx_rr(XD, XS)                                                    \
        anncx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negcs_rx(XG)                                                        \
        negcs_rr(W(XG), W(XG))

#define negcs_rr(XD, XS)                                                    \
        xorcx3ld(W(XD), W(XS), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addcs_rr(XG, XS)                                                    \
        addcs3rr(W(XG), W(XG), W(XS))

#define addcs_ld(XG, MS, DS)                                                \
        addcs3ld(W(XG), W(XG), W(MS), W(DS))

#define addcs3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subcs_rr(XG, XS)                                                    \
        subcs3rr(W(XG), W(XG), W(XS))

#define subcs_ld(XG, MS, DS)                                                \
        subcs3ld(W(XG), W(XG), W(MS), W(DS))

#define subcs3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulcs_rr(XG, XS)                                                    \
        mulcs3rr(W(XG), W(XG), W(XS))

#define mulcs_ld(XG, MS, DS)                                                \
        mulcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcs3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divcs_rr(XG, XS)                                                    \
        divcs3rr(W(XG), W(XG), W(XS))

#define divcs_ld(XG, MS, DS)                                                \
        divcs3ld(W(XG), W(XG), W(MS), W(DS))

#define divcs3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrcs_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0

#define rcecs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcscs_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RCP == 2

#define rcecs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcscs_rr(XG, XS) /* destroys XS */                                  \
        mulcs_rr(W(XS), W(XG))                                              \
        mulcs_rr(W(XS), W(XG))                                              \
        addcs_rr(W(XG), W(XG))                                              \
        subcs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0

#define rsecs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rsscs_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RSQ == 2

#define rsecs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rsscs_rr(XG, XS) /* destroys XS */                                  \
        mulcs_rr(W(XS), W(XG))                                              \
        mulcs_rr(W(XS), W(XG))                                              \
        subcs_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulcs_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulcs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
    ADR EVX(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmacs_ld(XG, XS, MT, DT)                                            \
    ADR EVX(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
    ADR EVX(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmscs_ld(XG, XS, MT, DT)                                            \
    ADR EVX(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define mincs_rr(XG, XS)                                                    \
        mincs3rr(W(XG), W(XG), W(XS))

#define mincs_ld(XG, MS, DS)                                                \
        mincs3ld(W(XG), W(XG), W(MS), W(DS))

#define mincs3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mincs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxcs_rr(XG, XS)                                                    \
        maxcs3rr(W(XG), W(XG), W(XS))

#define maxcs_ld(XG, MS, DS)                                                \
        maxcs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcs3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqcs_rr(XG, XS)                                                    \
        ceqcs3rr(W(XG), W(XG), W(XS))

#define ceqcs_ld(XG, MS, DS)                                                \
        ceqcs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

#define ceqcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnecs_rr(XG, XS)                                                    \
        cnecs3rr(W(XG), W(XG), W(XS))

#define cnecs_ld(XG, MS, DS)                                                \
        cnecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

#define cnecs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltcs_rr(XG, XS)                                                    \
        cltcs3rr(W(XG), W(XG), W(XS))

#define cltcs_ld(XG, MS, DS)                                                \
        cltcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

#define cltcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clecs_rr(XG, XS)                                                    \
        clecs3rr(W(XG), W(XG), W(XS))

#define clecs_ld(XG, MS, DS)                                                \
        clecs3ld(W(XG), W(XG), W(MS), W(DS))

#define clecs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

#define clecs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtcs_rr(XG, XS)                                                    \
        cgtcs3rr(W(XG), W(XG), W(XS))

#define cgtcs_ld(XG, MS, DS)                                                \
        cgtcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

#define cgtcs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgecs_rr(XG, XS)                                                    \
        cgecs3rr(W(XG), W(XG), W(XS))

#define cgecs_ld(XG, MS, DS)                                                \
        cgecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecs3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)

#define cgecs3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1cx_ld(W(XD), Mebp, inf_GPC07)


#define mz1cx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR EZX(RXB(XG), RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256    0xFF     /*  all satisfy the condition */

#define mk1wx_rx(RD)         /* not portable, do not use outside */         \
        VEX(RXB(RD),       0,    0x00, 0, 0, 1) EMITB(0x93)                 \
        MRM(REG(RD),    0x03,    0x01)

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1cx_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_256))                     \
        jeqxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        ERX(RXB(XD), RXB(XS),    0x00, 2, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        cvpcs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        ERX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        cvmcs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        cvtcs_rr(W(XD), W(XS))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        cvtcs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        cvtcn_rr(W(XD), W(XS))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        cvtcn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndcs_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtcs_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcs_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcn_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcn_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrcs_rr(XD, XS, mode)                                              \
        ERX(RXB(XD), RXB(XS), 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)\
        MRM(REG(XD), MOD(XS), REG(XS))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addcx_rr(XG, XS)                                                    \
        addcx3rr(W(XG), W(XG), W(XS))

#define addcx_ld(XG, MS, DS)                                                \
        addcx3ld(W(XG), W(XG), W(MS), W(DS))

#define addcx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subcx_rr(XG, XS)                                                    \
        subcx3rr(W(XG), W(XG), W(XS))

#define subcx_ld(XG, MS, DS)                                                \
        subcx3ld(W(XG), W(XG), W(MS), W(DS))

#define subcx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlcx_ri(XG, IS)                                                    \
        shlcx3ri(W(XG), W(XG), W(IS))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlcx3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))

#define shlcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcx_ri(XG, IS)                                                    \
        shrcx3ri(W(XG), W(XG), W(IS))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcx3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))

#define shrcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcn_ri(XG, IS)                                                    \
        shrcn3ri(W(XG), W(XG), W(IS))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcn3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))

#define shrcn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx3rr(W(XG), W(XG), W(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlcx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx3rr(W(XG), W(XG), W(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrcx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn3rr(W(XG), W(XG), W(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcn3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrcn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#define RT_SIMD_WIDTH32_512 16

#define muvox_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 2, 0, 1) EMITB(0x10)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define muvox_st(XS, MD, DD)                                                \
    ADR EVX(RXB(XS), RXB(MD),    0x00, 2, 0, 1) EMITB(0x11)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* sregs */

#undef  sregs_sa
#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        muvox_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmG, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmH, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmI, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmJ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmK, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmL, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmM, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmN, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmO, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmP, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmQ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmR, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmS, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_st(XmmT, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        VEX(0,             0,    0x00, 0, 0, 1) EMITB(0x91)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_st(Redx)                                                      \
        stack_st(Recx)                                                      \
        stack_st(Rebx)                                                      \
        stack_st(Reax)                                                      \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        stack_ld(Reax)                                                      \
        andwz_ri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
        EMITB(0x74) EMITB(0x05)                                             \
        VEW(0,             0,    0x00, 0, 0, 1) EMITB(0x91)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_ld(Rebx)                                                      \
        stack_ld(Recx)                                                      \
        stack_ld(Redx)

#undef  sregs_la
#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        muvox_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmG, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmH, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmI, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmJ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmK, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmL, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmM, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmN, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmO, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmP, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmQ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmR, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmS, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        muvox_ld(XmmT, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        VEX(0,             0,    0x00, 0, 0, 1) EMITB(0x90)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_st(Redx)                                                      \
        stack_st(Recx)                                                      \
        stack_st(Rebx)                                                      \
        stack_st(Reax)                                                      \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        stack_ld(Reax)                                                      \
        andwz_ri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
        EMITB(0x74) EMITB(0x05)                                             \
        VEW(0,             0,    0x00, 0, 0, 1) EMITB(0x90)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_ld(Rebx)                                                      \
        stack_ld(Recx)                                                      \
        stack_ld(Redx)

#endif /* RT_256X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_256X1V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

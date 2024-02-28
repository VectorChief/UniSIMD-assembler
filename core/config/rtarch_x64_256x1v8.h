/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_256X1V8_H
#define RT_RTARCH_X64_256X1V8_H

#include "rtarch_x32_256x1v8.h"
#include "rtarch_xHB_256x1v8.h"
#include "rtarch_xHF_256x1v8.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_256x1v8.h: Implementation of x86_64 fp64 AVX512VL/DQ instructions.
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

#if (defined RT_SIMD_CODE)

#if (RT_256X1 == 8)

#define ck1dx_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVW(0,       RXB(XS),    0x00, 1, 2, 2) EMITB(0x39)                 \
        MRM(0x01,    MOD(XS), REG(XS))

#define mz1dx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVW(RXB(XD),       0,    0x00, 1, 2, 2) EMITB(0x38)                 \
        MRM(REG(XD),    0x03,    0x01)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

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
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movdx_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movdx_st(XS, MD, DD)                                                \
    ADR EVW(RXB(XS), RXB(MD),    0x00, 1, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvdx_rr(XG, XS)                                                    \
        ck1dx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(RXB(XG), RXB(XS),    0x00, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvdx_ld(XG, MS, DS)                                                \
        ck1dx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XG), RXB(MS),    0x00, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvdx_st(XS, MG, DG)                                                \
        ck1dx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XS), RXB(MG),    0x00, 1, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define anddx_rr(XG, XS)                                                    \
        anddx3rr(W(XG), W(XG), W(XS))

#define anddx_ld(XG, MS, DS)                                                \
        anddx3ld(W(XG), W(XG), W(MS), W(DS))

#define anddx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anddx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anndx_rr(XG, XS)                                                    \
        anndx3rr(W(XG), W(XG), W(XS))

#define anndx_ld(XG, MS, DS)                                                \
        anndx3ld(W(XG), W(XG), W(MS), W(DS))

#define anndx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anndx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrdx_rr(XG, XS)                                                    \
        orrdx3rr(W(XG), W(XG), W(XS))

#define orrdx_ld(XG, MS, DS)                                                \
        orrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrdx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orndx_rr(XG, XS)                                                    \
        notdx_rx(W(XG))                                                     \
        orrdx_rr(W(XG), W(XS))

#define orndx_ld(XG, MS, DS)                                                \
        notdx_rx(W(XG))                                                     \
        orrdx_ld(W(XG), W(MS), W(DS))

#define orndx3rr(XD, XS, XT)                                                \
        notdx_rr(W(XD), W(XS))                                              \
        orrdx_rr(W(XD), W(XT))

#define orndx3ld(XD, XS, MT, DT)                                            \
        notdx_rr(W(XD), W(XS))                                              \
        orrdx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xordx_rr(XG, XS)                                                    \
        xordx3rr(W(XG), W(XG), W(XS))

#define xordx_ld(XG, MS, DS)                                                \
        xordx3ld(W(XG), W(XG), W(MS), W(DS))

#define xordx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xordx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notdx_rx(XG)                                                        \
        notdx_rr(W(XG), W(XG))

#define notdx_rr(XD, XS)                                                    \
        anndx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negds_rx(XG)                                                        \
        negds_rr(W(XG), W(XG))

#define negds_rr(XD, XS)                                                    \
        xordx3ld(W(XD), W(XS), Mebp, inf_GPC06_64)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addds_rr(XG, XS)                                                    \
        addds3rr(W(XG), W(XG), W(XS))

#define addds_ld(XG, MS, DS)                                                \
        addds3ld(W(XG), W(XG), W(MS), W(DS))

#define addds3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subds_rr(XG, XS)                                                    \
        subds3rr(W(XG), W(XG), W(XS))

#define subds_ld(XG, MS, DS)                                                \
        subds3ld(W(XG), W(XG), W(MS), W(DS))

#define subds3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulds_rr(XG, XS)                                                    \
        mulds3rr(W(XG), W(XG), W(XS))

#define mulds_ld(XG, MS, DS)                                                \
        mulds3ld(W(XG), W(XG), W(MS), W(DS))

#define mulds3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divds_rr(XG, XS)                                                    \
        divds3rr(W(XG), W(XG), W(XS))

#define divds_ld(XG, MS, DS)                                                \
        divds3ld(W(XG), W(XG), W(MS), W(DS))

#define divds3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqrds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrds_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0

#define rceds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsds_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RCP == 2

#define rceds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsds_rr(XG, XS) /* destroys XS */                                  \
        mulds_rr(W(XS), W(XG))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        addds_rr(W(XG), W(XG))                                              \
        subds_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0

#define rseds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssds_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RSQ == 2

#define rseds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssds_rr(XG, XS) /* destroys XS */                                  \
        mulds_rr(W(XS), W(XG))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        subds_ld(W(XS), Mebp, inf_GPC03_64)                                 \
        mulds_ld(W(XS), Mebp, inf_GPC02_64)                                 \
        mulds_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmads_rr(XG, XS, XT)                                                \
    ADR EVW(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmads_ld(XG, XS, MT, DT)                                            \
    ADR EVW(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsds_rr(XG, XS, XT)                                                \
    ADR EVW(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsds_ld(XG, XS, MT, DT)                                            \
    ADR EVW(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minds_rr(XG, XS)                                                    \
        minds3rr(W(XG), W(XG), W(XS))

#define minds_ld(XG, MS, DS)                                                \
        minds3ld(W(XG), W(XG), W(MS), W(DS))

#define minds3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxds_rr(XG, XS)                                                    \
        maxds3rr(W(XG), W(XG), W(XS))

#define maxds_ld(XG, MS, DS)                                                \
        maxds3ld(W(XG), W(XG), W(MS), W(DS))

#define maxds3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqds_rr(XG, XS)                                                    \
        ceqds3rr(W(XG), W(XG), W(XS))

#define ceqds_ld(XG, MS, DS)                                                \
        ceqds3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqds3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define ceqds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneds_rr(XG, XS)                                                    \
        cneds3rr(W(XG), W(XG), W(XS))

#define cneds_ld(XG, MS, DS)                                                \
        cneds3ld(W(XG), W(XG), W(MS), W(DS))

#define cneds3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cneds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltds_rr(XG, XS)                                                    \
        cltds3rr(W(XG), W(XG), W(XS))

#define cltds_ld(XG, MS, DS)                                                \
        cltds3ld(W(XG), W(XG), W(MS), W(DS))

#define cltds3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cltds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleds_rr(XG, XS)                                                    \
        cleds3rr(W(XG), W(XG), W(XS))

#define cleds_ld(XG, MS, DS)                                                \
        cleds3ld(W(XG), W(XG), W(MS), W(DS))

#define cleds3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cleds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtds_rr(XG, XS)                                                    \
        cgtds3rr(W(XG), W(XG), W(XS))

#define cgtds_ld(XG, MS, DS)                                                \
        cgtds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtds3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cgtds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeds_rr(XG, XS)                                                    \
        cgeds3rr(W(XG), W(XG), W(XS))

#define cgeds_ld(XG, MS, DS)                                                \
        cgeds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeds3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cgeds3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_256    0x0F     /*  all satisfy the condition */

/* #define mk1wx_rx(RD)                    (defined in 32_256-bit header) */

#define mkjdx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1dx_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##64_256))                     \
        jeqxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzds_rr(XD, XS)     /* round towards zero */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzds_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x7A)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        ERW(RXB(XD), RXB(XS),    0x00, 2, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        movds_ld(W(XD), W(MS), W(DS))                                       \
        cvpds_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        ERW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        movds_ld(W(XD), W(MS), W(DS))                                       \
        cvmds_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnds_rr(XD, XS)     /* round towards near */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvnds_rr(XD, XS)     /* round towards near */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvnds_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndds_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtds_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrds_rr(XD, XS, mode)                                              \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrds_rr(XD, XS, mode)                                              \
        ERW(RXB(XD), RXB(XS), 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x7B)\
        MRM(REG(XD), MOD(XS), REG(XS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndn_rr(XD, XS)     /* round towards near */                       \
        cvtdn_rr(W(XD), W(XS))

#define cvndn_ld(XD, MS, DS) /* round towards near */                       \
        cvtdn_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdn_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtdn_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndx_rr(XD, XS)     /* round towards near */                       \
        cvtdx_rr(W(XD), W(XS))

#define cvndx_ld(XD, MS, DS) /* round towards near */                       \
        cvtdx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdx_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 2, 1) EMITB(0x7A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtdx_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 2, 1) EMITB(0x7A)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit unsigned int range */

#define ruzds_rr(XD, XS)     /* round towards zero */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define ruzds_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cuzds_rr(XD, XS)     /* round towards zero */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x78)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cuzds_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x78)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit unsigned int range */

#define rupds_rr(XD, XS)     /* round towards +inf */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rupds_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cupds_rr(XD, XS)     /* round towards +inf */                       \
        ERW(RXB(XD), RXB(XS),    0x00, 2, 1, 1) EMITB(0x79)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cupds_ld(XD, MS, DS) /* round towards +inf */                       \
        movds_ld(W(XD), W(MS), W(DS))                                       \
        cupds_rr(W(XD), W(XD))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit unsigned int range */

#define rumds_rr(XD, XS)     /* round towards -inf */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rumds_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cumds_rr(XD, XS)     /* round towards -inf */                       \
        ERW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x79)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cumds_ld(XD, MS, DS) /* round towards -inf */                       \
        movds_ld(W(XD), W(MS), W(DS))                                       \
        cumds_rr(W(XD), W(XD))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit unsigned int range */

#define runds_rr(XD, XS)     /* round towards near */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define runds_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cunds_rr(XD, XS)     /* round towards near */                       \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x79)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cunds_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x79)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit unsigned int range */

#define rudds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rudds_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cutds_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x79)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cutds_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x79)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit unsigned int range */

#define rurds_rr(XD, XS, mode)                                              \
        EVW(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define curds_rr(XD, XS, mode)                                              \
        ERW(RXB(XD), RXB(XS), 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x79)\
        MRM(REG(XD), MOD(XS), REG(XS))

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define adddx_rr(XG, XS)                                                    \
        adddx3rr(W(XG), W(XG), W(XS))

#define adddx_ld(XG, MS, DS)                                                \
        adddx3ld(W(XG), W(XG), W(MS), W(DS))

#define adddx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adddx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subdx_rr(XG, XS)                                                    \
        subdx3rr(W(XG), W(XG), W(XS))

#define subdx_ld(XG, MS, DS)                                                \
        subdx3ld(W(XG), W(XG), W(MS), W(DS))

#define subdx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define muldx_rr(XG, XS)                                                    \
        muldx3rr(W(XG), W(XG), W(XS))

#define muldx_ld(XG, MS, DS)                                                \
        muldx3ld(W(XG), W(XG), W(MS), W(DS))

#define muldx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define muldx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shldx_ri(XG, IS)                                                    \
        shldx3ri(W(XG), W(XG), W(IS))

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx3ld(W(XG), W(XG), W(MS), W(DS))

#define shldx3ri(XD, XS, IT)                                                \
        EVW(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shldx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdx_ri(XG, IS)                                                    \
        shrdx3ri(W(XG), W(XG), W(IS))

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdx3ri(XD, XS, IT)                                                \
        EVW(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdn_ri(XG, IS)                                                    \
        shrdn3ri(W(XG), W(XG), W(IS))

#define shrdn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdn3ri(XD, XS, IT)                                                \
        EVW(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrdn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx3rr(W(XG), W(XG), W(XS))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx3ld(W(XG), W(XG), W(MS), W(DS))

#define svldx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svldx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx3rr(W(XG), W(XG), W(XS))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn3rr(W(XG), W(XG), W(XS))

#define svrdn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdn3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrdn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mindx_rr(XG, XS)                                                    \
        mindx3rr(W(XG), W(XG), W(XS))

#define mindx_ld(XG, MS, DS)                                                \
        mindx3ld(W(XG), W(XG), W(MS), W(DS))

#define mindx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mindx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mindn_rr(XG, XS)                                                    \
        mindn3rr(W(XG), W(XG), W(XS))

#define mindn_ld(XG, MS, DS)                                                \
        mindn3ld(W(XG), W(XG), W(MS), W(DS))

#define mindn3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mindn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxdx_rr(XG, XS)                                                    \
        maxdx3rr(W(XG), W(XG), W(XS))

#define maxdx_ld(XG, MS, DS)                                                \
        maxdx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxdx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxdn_rr(XG, XS)                                                    \
        maxdn3rr(W(XG), W(XG), W(XS))

#define maxdn_ld(XG, MS, DS)                                                \
        maxdn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxdn3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxdn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqdx_rr(XG, XS)                                                    \
        ceqdx3rr(W(XG), W(XG), W(XS))

#define ceqdx_ld(XG, MS, DS)                                                \
        ceqdx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqdx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define ceqdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnedx_rr(XG, XS)                                                    \
        cnedx3rr(W(XG), W(XG), W(XS))

#define cnedx_ld(XG, MS, DS)                                                \
        cnedx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnedx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cnedx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltdx_rr(XG, XS)                                                    \
        cltdx3rr(W(XG), W(XG), W(XS))

#define cltdx_ld(XG, MS, DS)                                                \
        cltdx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltdx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cltdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltdn_rr(XG, XS)                                                    \
        cltdn3rr(W(XG), W(XG), W(XS))

#define cltdn_ld(XG, MS, DS)                                                \
        cltdn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltdn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cltdn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cledx_rr(XG, XS)                                                    \
        cledx3rr(W(XG), W(XG), W(XS))

#define cledx_ld(XG, MS, DS)                                                \
        cledx3ld(W(XG), W(XG), W(MS), W(DS))

#define cledx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cledx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cledn_rr(XG, XS)                                                    \
        cledn3rr(W(XG), W(XG), W(XS))

#define cledn_ld(XG, MS, DS)                                                \
        cledn3ld(W(XG), W(XG), W(MS), W(DS))

#define cledn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cledn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtdx_rr(XG, XS)                                                    \
        cgtdx3rr(W(XG), W(XG), W(XS))

#define cgtdx_ld(XG, MS, DS)                                                \
        cgtdx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtdx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cgtdx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtdn_rr(XG, XS)                                                    \
        cgtdn3rr(W(XG), W(XG), W(XS))

#define cgtdn_ld(XG, MS, DS)                                                \
        cgtdn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtdn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cgtdn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgedx_rr(XG, XS)                                                    \
        cgedx3rr(W(XG), W(XG), W(XS))

#define cgedx_ld(XG, MS, DS)                                                \
        cgedx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgedx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cgedx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgedn_rr(XG, XS)                                                    \
        cgedn3rr(W(XG), W(XG), W(XS))

#define cgedn_ld(XG, MS, DS)                                                \
        cgedn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgedn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

#define cgedn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 1, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1dx_ld(W(XD), Mebp, inf_GPC07)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_256X1V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

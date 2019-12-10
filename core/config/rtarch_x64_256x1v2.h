/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_256X1V2_H
#define RT_RTARCH_X64_256X1V2_H

#include "rtarch_x32_256x1v2.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_256x1v2.h: Implementation of x86_64 fp64 AVX1/2 instructions.
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

#if (RT_256X1 >= 1 && RT_256X1 <= 2)

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
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movdx_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movdx_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),    0x00, 1, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvdx_rr(XG, XS)                                                    \
    ADR VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 3) EMITB(0x4B)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define mmvdx_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 3) EMITB(0x4B)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define mmvdx_st(XS, MG, DG)                                                \
    ADR VEX(RXB(XS), RXB(MG),    0x00, 1, 1, 2) EMITB(0x2F)                 \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define anddx_rr(XG, XS)                                                    \
        anddx3rr(W(XG), W(XG), W(XS))

#define anddx_ld(XG, MS, DS)                                                \
        anddx3ld(W(XG), W(XG), W(MS), W(DS))

#define anddx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anddx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define anndx_rr(XG, XS)                                                    \
        anndx3rr(W(XG), W(XG), W(XS))

#define anndx_ld(XG, MS, DS)                                                \
        anndx3ld(W(XG), W(XG), W(MS), W(DS))

#define anndx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anndx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrdx_rr(XG, XS)                                                    \
        orrdx3rr(W(XG), W(XG), W(XS))

#define orrdx_ld(XG, MS, DS)                                                \
        orrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrdx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrdx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xordx_rr(XG, XS)                                                    \
        xordx3rr(W(XG), W(XG), W(XS))

#define xordx_ld(XG, MS, DS)                                                \
        xordx3ld(W(XG), W(XG), W(MS), W(DS))

#define xordx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xordx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x57)                 \
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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addds_rr(XG, XS)                                                    \
        addds3rr(W(XG), W(XG), W(XS))

#define addds_ld(XG, MS, DS)                                                \
        addds3ld(W(XG), W(XG), W(MS), W(DS))

#define addds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subds_rr(XG, XS)                                                    \
        subds3rr(W(XG), W(XG), W(XS))

#define subds_ld(XG, MS, DS)                                                \
        subds3ld(W(XG), W(XG), W(MS), W(DS))

#define subds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulds_rr(XG, XS)                                                    \
        mulds3rr(W(XG), W(XG), W(XS))

#define mulds_ld(XG, MS, DS)                                                \
        mulds3ld(W(XG), W(XG), W(MS), W(DS))

#define mulds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divds_rr(XG, XS)                                                    \
        divds3rr(W(XG), W(XG), W(XS))

#define divds_ld(XG, MS, DS)                                                \
        divds3ld(W(XG), W(XG), W(MS), W(DS))

#define divds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqrds_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrds_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceds_rr(XD, XS)                                                    \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_ld(W(XD), Mebp, inf_SCR01(0))

#define rcsds_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseds_rr(XD, XS)                                                    \
        sqrds_rr(W(XD), W(XS))                                              \
        movdx_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_ld(W(XD), Mebp, inf_SCR01(0))

#define rssds_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_256X1 < 2) && !(RT_SIMD == 128 && RT_128X1 == 16)

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmads_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_rr(W(XS), W(XT))                                              \
        addds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmads_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_ld(W(XS), W(MT), W(DT))                                       \
        addds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmads_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmads_rx(W(XG))

#define fmads_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmads_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmads_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmads_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmads_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmads_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmads_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        movdx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x18))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x10))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movdx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsds_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_rr(W(XS), W(XT))                                              \
        subds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_ld(W(XS), W(MT), W(DT))                                       \
        subds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsds_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsds_rx(W(XG))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsds_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsds_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsds_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsds_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsds_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        movdx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x18))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x10))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movdx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_256X1 >= 2 || RT_SIMD == 128 && RT_128X1 == 16, AVX2 or FMA3 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmads_rr(XG, XS, XT)                                                \
    ADR VEW(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmads_ld(XG, XS, MT, DT)                                            \
    ADR VEW(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsds_rr(XG, XS, XT)                                                \
    ADR VEW(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsds_ld(XG, XS, MT, DT)                                            \
    ADR VEW(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_256X1 >= 2 || RT_SIMD == 128 && RT_128X1 == 16, AVX2 or FMA3 */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minds_rr(XG, XS)                                                    \
        minds3rr(W(XG), W(XG), W(XS))

#define minds_ld(XG, MS, DS)                                                \
        minds3ld(W(XG), W(XG), W(MS), W(DS))

#define minds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxds_rr(XG, XS)                                                    \
        maxds3rr(W(XG), W(XG), W(XS))

#define maxds_ld(XG, MS, DS)                                                \
        maxds3ld(W(XG), W(XG), W(MS), W(DS))

#define maxds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqds_rr(XG, XS)                                                    \
        ceqds3rr(W(XG), W(XG), W(XS))

#define ceqds_ld(XG, MS, DS)                                                \
        ceqds3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneds_rr(XG, XS)                                                    \
        cneds3rr(W(XG), W(XG), W(XS))

#define cneds_ld(XG, MS, DS)                                                \
        cneds3ld(W(XG), W(XG), W(MS), W(DS))

#define cneds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltds_rr(XG, XS)                                                    \
        cltds3rr(W(XG), W(XG), W(XS))

#define cltds_ld(XG, MS, DS)                                                \
        cltds3ld(W(XG), W(XG), W(MS), W(DS))

#define cltds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleds_rr(XG, XS)                                                    \
        cleds3rr(W(XG), W(XG), W(XS))

#define cleds_ld(XG, MS, DS)                                                \
        cleds3ld(W(XG), W(XG), W(MS), W(DS))

#define cleds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtds_rr(XG, XS)                                                    \
        cgtds3rr(W(XG), W(XG), W(XS))

#define cgtds_ld(XG, MS, DS)                                                \
        cgtds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeds_rr(XG, XS)                                                    \
        cgeds3rr(W(XG), W(XG), W(XS))

#define cgeds_ld(XG, MS, DS)                                                \
        cgeds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeds3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeds3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_256    0xFF     /*  all satisfy the condition */

#define mkjdx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        VEX(0,       RXB(XS),    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##64_256))                     \
        jeqxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzds_rr(XD, XS)     /* round towards zero */                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x18))                                    \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvzds_ld(XD, MS, DS) /* round towards zero */                       \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        rnpds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        rnmds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnds_rr(XD, XS)     /* round towards near */                       \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvnds_rr(XD, XS)     /* round towards near */                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x18))                                    \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnds_ld(XD, MS, DS) /* round towards near */                       \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvnds_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndn_rr(XD, XS)     /* round towards near */                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x18))                                    \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvndn_ld(XD, MS, DS) /* round towards near */                       \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvndn_rr(W(XD), W(XD))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndds_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtds_rr(XD, XS)                                                    \
        rndds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvtds_ld(XD, MS, DS)                                                \
        rndds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdn_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvndn_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtdn_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvtdn_rr(W(XD), W(XD))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrds_rr(XD, XS, mode)                                              \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrds_rr(XD, XS, mode)                                              \
        rnrds_rr(W(XD), W(XS), mode)                                        \
        cvzds_rr(W(XD), W(XD))

/************   packed double-precision integer arithmetic/shifts   ***********/

#if (RT_256X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define adddx_rr(XG, XS)                                                    \
        adddx3rr(W(XG), W(XG), W(XS))

#define adddx_ld(XG, MS, DS)                                                \
        adddx3ld(W(XG), W(XG), W(MS), W(DS))

#define adddx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addjx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define adddx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addjx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subdx_rr(XG, XS)                                                    \
        subdx3rr(W(XG), W(XG), W(XS))

#define subdx_ld(XG, MS, DS)                                                \
        subdx3ld(W(XG), W(XG), W(MS), W(DS))

#define subdx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subjx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define subdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subjx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define muldx_rr(XG, XS)                                                    \
        muldx3rr(W(XG), W(XG), W(XS))

#define muldx_ld(XG, MS, DS)                                                \
        muldx3ld(W(XG), W(XG), W(MS), W(DS))

#define muldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
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

#define muldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
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

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shldx_ri(XG, IS)                                                    \
        shldx3ri(W(XG), W(XG), W(IS))

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx3ld(W(XG), W(XG), W(MS), W(DS))

#define shldx3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shljx3ri(W(XD), W(XS), W(IT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shljx_ri(W(XD), W(IT))                                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shljx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shljx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdx_ri(XG, IS)                                                    \
        shrdx3ri(W(XG), W(XG), W(IS))

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdx3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrjx3ri(W(XD), W(XS), W(IT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrjx_ri(W(XD), W(IT))                                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrjx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx3rr(W(XG), W(XG), W(XS))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx3ld(W(XG), W(XG), W(MS), W(DS))

#define svldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
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

#define svldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
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

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx3rr(W(XG), W(XG), W(XS))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
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

#define svrdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
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

#else /* RT_256X1 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define adddx_rr(XG, XS)                                                    \
        adddx3rr(W(XG), W(XG), W(XS))

#define adddx_ld(XG, MS, DS)                                                \
        adddx3ld(W(XG), W(XG), W(MS), W(DS))

#define adddx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adddx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subdx_rr(XG, XS)                                                    \
        subdx3rr(W(XG), W(XG), W(XS))

#define subdx_ld(XG, MS, DS)                                                \
        subdx3ld(W(XG), W(XG), W(MS), W(DS))

#define subdx3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subdx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define muldx_rr(XG, XS)                                                    \
        muldx3rr(W(XG), W(XG), W(XS))

#define muldx_ld(XG, MS, DS)                                                \
        muldx3ld(W(XG), W(XG), W(MS), W(DS))

#define muldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
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

#define muldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
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

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shldx_ri(XG, IS)                                                    \
        shldx3ri(W(XG), W(XG), W(IS))

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx3ld(W(XG), W(XG), W(MS), W(DS))

#define shldx3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x3F))

#define shldx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdx_ri(XG, IS)                                                    \
        shrdx3ri(W(XG), W(XG), W(IS))

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdx3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x3F))

#define shrdx3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx3rr(W(XG), W(XG), W(XS))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx3ld(W(XG), W(XG), W(MS), W(DS))

#define svldx3rr(XD, XS, XT)                                                \
        VEW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svldx3ld(XD, XS, MT, DT)                                            \
    ADR VEW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx3rr(W(XG), W(XG), W(XS))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdx3rr(XD, XS, XT)                                                \
        VEW(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrdx3ld(XD, XS, MT, DT)                                            \
    ADR VEW(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
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
        movzx_ld(Recx, W(MT), W(DT))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn3rr(W(XG), W(XG), W(XS))

#define svrdn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdn3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
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

#define svrdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
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

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_256X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

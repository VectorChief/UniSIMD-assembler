/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_256_H
#define RT_RTARCH_X64_256_H

#include "rtarch_x32_256.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_256.h: Implementation of x86_64 fp64 AVX(1,2) instructions.
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
 * cmdpx_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmdpn_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmdps_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD register/memory/immediate args
 * cmdp*_** - applies [cmd] to L-size SIMD register/memory/immediate args
 * cmdq*_** - applies [cmd] to 64-bit SIMD register/memory/immediate args
 *
 * The cmdp*_** (rtbase.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 * Matching element-sized BASE subset cmdy*_** is defined in rtbase.h.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and fisrt source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and fisrt source
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

#if defined (RT_SIMD_CODE)

#if defined (RT_256) && (RT_256 != 0) || defined (RT_128) && (RT_128 >= 8)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov (D = S) */

#define movqx_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movqx_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movqx_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),    0x00, K, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* and (G = G & S) */

#define andqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andqx_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* ann (G = ~G & S) */

#define annqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annqx_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orr (G = G | S) */

#define orrqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrqx_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orn (G = ~G | S) */

#define ornqx_rr(XG, XS)                                                    \
        notqx_rx(W(XG))                                                     \
        orrqx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        notqx_rx(W(XG))                                                     \
        orrqx_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorqx_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* not (G = ~G) */

#define notqx_rx(XG)                                                        \
        annqx_ld(W(XG), Mebp, inf_GPC07)

/**************   packed double precision floating point (AVX1)   *************/

/* neg (G = -G) */

#define negqs_rx(XG)                                                        \
        xorqx_ld(W(XG), Mebp, inf_GPC06_64)

/* add (G = G + S) */

#define addqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub (G = G - S) */

#define subqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* mul (G = G * S) */

#define mulqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* div (G = G / S) */

#define divqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sqr (D = sqrt S) */

#define sqrqs_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrqs_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceqs_rr(XD, XS)                                                    \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR01(0))

#define rcsqs_rr(XG, XS) /* destroys MS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseqs_rr(XD, XS)                                                    \
        sqrqs_rr(W(XD), W(XS))                                              \
        movqx_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR01(0))

#define rssqs_rr(XG, XS) /* destroys MS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if defined (RT_256) && (RT_256 < 2) || \
    defined (RT_128) && (RT_SIMD_COMPAT_128 == 1)

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaqs_rr(XG, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_rr(W(XS), W(XT))                                              \
        addqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_ld(W(XS), W(MT), W(DT))                                       \
        addqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmaqs_rr(XG, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmaqs_rx(W(XG))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XS), W(MT), W(DT))                                       \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmaqs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmaqs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmaqs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XS), W(MT), W(DT))                                       \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmaqs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#if defined (RT_256) && (RT_256 < 2) /* NOTE: x87 fallback for fp64 FMA */

#define fmaqs_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x18))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x10))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movqx_ld(W(XG), Mebp, inf_SCR02(0))

#else /* RT_128 >= 8 */ /* NOTE: x87 fallback for fp64 FMA (128-bit AVX1) */

#define fmaqs_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movqx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_128 >= 8 */

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsqs_rr(XG, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_rr(W(XS), W(XT))                                              \
        subqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulqs_ld(W(XS), W(MT), W(DT))                                       \
        subqs_rr(W(XG), W(XS))                                              \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsqs_rr(XG, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsqs_rx(W(XG))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XS), W(MT), W(DT))                                       \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsqs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsqs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsqs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XS), W(MT), W(DT))                                       \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsqs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#if defined (RT_256) && (RT_256 < 2) /* NOTE: x87 fallback for fp64 FMS */

#define fmsqs_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x18))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x10))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movqx_ld(W(XG), Mebp, inf_SCR02(0))

#else /* RT_128 >= 8 */ /* NOTE: x87 fallback for fp64 FMS (128-bit AVX1) */

#define fmsqs_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movqx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_128 >= 8 */

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_256 >= 2 */ /* NOTE: FMA is available in processors with AVX2 */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaqs_rr(XG, XS, XT)                                                \
    ADR VEW(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
    ADR VEW(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsqs_rr(XG, XS, XT)                                                \
    ADR VEW(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
    ADR VEW(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_256 >= 2 */

/* min (G = G < S ? G : S) */

#define minqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* max (G = G > S ? G : S) */

#define maxqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cmp (G = G ? S) */

#define ceqqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cneqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cltqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cleqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cgtqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgeqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeqs_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzqs_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x18))                                    \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpqs_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        rnpqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XD, XS)     /* round towards -inf */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmqs_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        rnmqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XD, XS)     /* round towards near */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnqs_ld(XD, MS, DS) /* round towards near */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x18))                                    \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnqs_ld(XD, MS, DS) /* round towards near */                       \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvnqs_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x18))                                    \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnqn_ld(XD, MS, DS) /* round towards near */                       \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvnqn_rr(W(XD), W(XD))

/**************************   extended float (x87)   **************************/

#define fpuzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDD)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuzs_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDD)                                   \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpuzn_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDF)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuzn_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDF)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#define fpuzt_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDD)                                   \
        MRM(0x01,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define addzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbrzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x01,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define dvrzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define addes_xn(n)      /* ST(0) = ST(0) + ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xC0+(n))

#define addes_nx(n)      /* ST(n) = ST(n) + ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xC0+(n))

#define addes_np(n)      /* ST(n) = ST(n) + ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xC0+(n))


#define subes_xn(n)      /* ST(0) = ST(0) - ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xE0+(n))

#define subes_nx(n)      /* ST(n) = ST(n) - ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xE8+(n))

#define subes_np(n)      /* ST(n) = ST(n) - ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xE8+(n))


#define sbres_xn(n)      /* ST(0) = ST(n) - ST(0), don't pop */             \
        EMITB(0xD8) EMITB(0xE8+(n))

#define sbres_nx(n)      /* ST(n) = ST(0) - ST(n), don't pop */             \
        EMITB(0xDC) EMITB(0xE0+(n))

#define sbres_np(n)      /* ST(n) = ST(0) - ST(n), pop stack */             \
        EMITB(0xDE) EMITB(0xE0+(n))


#define mules_xn(n)      /* ST(0) = ST(0) * ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xC8+(n))

#define mules_nx(n)      /* ST(n) = ST(n) * ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xC8+(n))

#define mules_np(n)      /* ST(n) = ST(n) * ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xC8+(n))


#define dives_xn(n)      /* ST(0) = ST(0) / ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xF0+(n))

#define dives_nx(n)      /* ST(n) = ST(n) / ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xF8+(n))

#define dives_np(n)      /* ST(n) = ST(n) / ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xF8+(n))


#define dvres_xn(n)      /* ST(0) = ST(n) / ST(0), don't pop */             \
        EMITB(0xD8) EMITB(0xF8+(n))

#define dvres_nx(n)      /* ST(n) = ST(0) / ST(n), don't pop */             \
        EMITB(0xDC) EMITB(0xF0+(n))

#define dvres_np(n)      /* ST(n) = ST(0) / ST(n), pop stack */             \
        EMITB(0xDE) EMITB(0xF0+(n))


#define sqres_xx()       /* ST(0) = sqr ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xFA)

#define neges_xx()       /* ST(0) = neg ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xE0)

#define abses_xx()       /* ST(0) = abs ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xE1)


#define xm2es_xx()       /* ST(0) = 2^ST(0)-1, don't pop, [-1.0 : +1.0] */  \
        EMITB(0xD9) EMITB(0xF0)

#define lg2es_xx()       /* ST(1) = ST(1)*lg2 ST(0), pop stack */           \
        EMITB(0xD9) EMITB(0xF1)

#define lp2es_xx()       /* ST(1) = ST(1)*lg2 ST(0)+1.0, pop stack */       \
        EMITB(0xD9) EMITB(0xF9)


#define sines_xx()       /* ST(0) = sin ST(0), don't pop, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xFE)

#define coses_xx()       /* ST(0) = cos ST(0), don't pop, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xFF)

#define scses_xx()       /* ST(0) = sin ST(0), push cos ST(0), original */  \
        EMITB(0xD9) EMITB(0xFB)

#define tanes_xx()       /* ST(0) = tan ST(0), push +1.0, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xF2)

#define atnes_xx()       /* ST(1) = atn ST(1)/ST(0), pop stack */           \
        EMITB(0xD9) EMITB(0xF3)


#define remes_xx()       /* ST(0) = ST(0)-Q*ST(1), Q = rnd ST(0)/ST(1) */   \
        EMITB(0xD9) EMITB(0xF5)

#define rexes_xx()       /* ST(0) = ST(0)-Q*ST(1), Q = trn ST(0)/ST(1) */   \
        EMITB(0xD9) EMITB(0xF8)

#define rndes_xx()       /* ST(0) = rnd ST(0), round to integral value */   \
        EMITB(0xD9) EMITB(0xFC)

#define extes_xx()       /* ST(0) = exp ST(0), push mts ST(0) */            \
        EMITB(0xD9) EMITB(0xF4)

#define scles_xx()       /* exp ST(0) = exp ST(0) + rnd ST(1) */             \
        EMITB(0xD9) EMITB(0xFD)


#define cmpes_xn(n)      /* flags = ST(0) ? ST(n), don't pop */             \
        EMITB(0xDB) EMITB(0xF0+(n))

#define cmpes_pn(n)      /* flags = ST(0) ? ST(n), pop stack */             \
        EMITB(0xDF) EMITB(0xF0+(n))

#define moves_nx(n)      /* ST(n) = ST(0), don't pop */                     \
        EMITB(0xDD) EMITB(0xD0+(n))

#define moves_np(n)      /* ST(n) = ST(0), pop stack */                     \
        EMITB(0xDD) EMITB(0xD8+(n))

#define popes_xx()       /* ST(0) = ST(0), pop stack */                     \
        EMITB(0xDD) EMITB(0xD8)


#define fpucw_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD9)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpucw_st(MD, DD) /* not portable, do not use outside */             \
FWT ADR REX(0,       RXB(MD)) EMITB(0xD9)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpurz_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR02(4))

/**************************   packed integer (AVX1)   *************************/

#if defined (RT_256) && (RT_256 < 2)

#define prmqx_rr(XD, XS, IT) /* not portable, do not use outside */         \
        VEX(RXB(XD), RXB(XS), REN(XD), K, 1, 3) EMITB(0x06)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define movjx_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 0, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movjx_st(XS, MD, DD) /* not portable, do not use outside */         \
    ADR VEX(RXB(XS), RXB(MD),    0x00, 0, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S) */

#define addjx_rr(XG, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 0, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addqx_rr(XG, XS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        addjx_rr(W(XG), W(XS))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmqx_rr(W(XS), W(XS), IB(1))                                       \
        addjx_rr(W(XG), W(XS))                                              \
        prmqx_rr(W(XS), W(XS), IB(1))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define addjx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addqx_ld(XG, MS, DS)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(MS), W(DS))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addjx_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addjx_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* sub (G = G - S) */

#define subjx_rr(XG, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 0, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subqx_rr(XG, XS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        subjx_rr(W(XG), W(XS))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmqx_rr(W(XS), W(XS), IB(1))                                       \
        subjx_rr(W(XG), W(XS))                                              \
        prmqx_rr(W(XS), W(XS), IB(1))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define subjx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subqx_ld(XG, MS, DS)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(MS), W(DS))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subjx_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subjx_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shl (G = G << S) */

#define shljx_ri(XG, IS)     /* not portable, do not use outside */         \
        VEX(0,       RXB(XG), REN(XG), 0, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shlqx_ri(XG, IS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shljx_ri(W(XG), W(IS))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shljx_ri(W(XG), W(IS))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shljx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shljx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shljx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shr (G = G >> S) */

#define shrjx_ri(XG, IS)     /* not portable, do not use outside */         \
        VEX(0,       RXB(XG), REN(XG), 0, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrqx_ri(XG, IS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrjx_ri(W(XG), W(IS))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrjx_ri(W(XG), W(IS))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrjx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_256 >= 2 */

/* add (G = G + S) */

#define addqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addqx_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub (G = G - S) */

#define subqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subqx_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shl (G = G << S) */

#define shlqx_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shr (G = G >> S) */

#define shrqx_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_256 >= 2 */

#if   defined (RT_128) && (RT_128 >= 8)

#define shrqn_ri(XG, IS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DS))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#elif defined (RT_256) && (RT_256 != 0)

#define shrqn_ri(XG, IS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x10), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x18), W(IS))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DS))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#endif /* RT_128, RT_256 */

/**************************   helper macros (AVX1)   **************************/

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndqs_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtqs_rr(XD, XS)                                                    \
        rndqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvtqs_ld(XD, MS, DS)                                                \
        rndqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtqn_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnqn_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtqn_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvtqn_rr(W(XD), W(XD))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrqs_rr(XD, XS, mode)                                              \
        rnrqs_rr(W(XD), W(XS), mode)                                        \
        cvzqs_rr(W(XD), W(XD))

/* mmv (D = mask-merge S)
 * uses Xmm0 implicitly as a mask register */

#define mmvqx_ld(XD, MS, DS) /* not portable, use conditionally (on x86) */ \
    ADR VEX(RXB(XD), RXB(MS), REN(XD), K, 1, 3) EMITB(0x4B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define mmvqx_st(XS, MD, DD) /* not portable, use conditionally (on x86) */ \
    ADR VEX(RXB(XS), RXB(MD),    0x00, K, 1, 2) EMITB(0x2F)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256, RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

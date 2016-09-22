/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_256_H
#define RT_RTARCH_X64_256_H

#include "rtarch_x32_256.h"

#if defined (RT_SIMD_CODE)

#if defined (RT_256) && (RT_256 != 0)

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
 * The cmdp*_** instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XG - SIMD register serving as target and fisrt source
 * XS - SIMD register serving as second source
 * IM - immediate value (smallest size IC is used for shifts)
 *
 * RG - BASE register serving as target and first source
 * RM - BASE register addressing mode (Oeax, M***, I***)
 * DP - displacement value (of given size DP, DF, DG, DH, DV)
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov */

#define movqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define movqx_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movqx_st(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x29)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andqx_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann (~XG & XS) */

#define annqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annqx_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrqx_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orn (~XG | XS) */

#define ornqx_rr(XG, XS)                                                    \
        notqx_rx(W(XG))                                                     \
        orrqx_rr(W(XG), W(XS))

#define ornqx_ld(XG, RM, DP)                                                \
        notqx_rx(W(XG))                                                     \
        orrqx_ld(W(XG), W(RM), W(DP))

/* xor */

#define xorqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorqx_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not */

#define notqx_rx(XG)                                                        \
        annqx_ld(W(XG), Mebp, inf_GPC07)

/**************   packed double precision floating point (AVX1)   *************/

/* neg */

#define negqs_rx(XG)                                                        \
        xorqx_ld(W(XG), Mebp, inf_GPC06_64)

/* add */

#define addqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqrqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sqrqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x51)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(XG, XS)                                                    \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XG), Mebp, inf_SCR01(0))

#define rcsqs_rr(XG, XS) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(XG, XS)                                                    \
        sqrqs_rr(W(XG), W(XS))                                              \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XG), Mebp, inf_SCR01(0))

#define rssqs_rr(XG, XS) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/**************************   packed integer (x87)   **************************/

#define fpuzs_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDD)                                   \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuzs_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDD)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuzn_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDF)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuzn_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDF)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuzt_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDD)                                   \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_st(RM, DP) /* not portable, do not use outside */             \
FWT ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XG, XS)     /* round towards zero */                       \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzqs_ld(XG, RM, DP) /* round towards zero */                       \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x03))

#define cvzqs_rr(XG, XS)     /* round towards zero */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x18))                                    \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define cvzqs_ld(XG, RM, DP) /* round towards zero */                       \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        cvzqs_rr(W(XG), W(XG))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XG, XS)     /* round towards +inf */                       \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpqs_ld(XG, RM, DP) /* round towards +inf */                       \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cvpqs_rr(XG, XS)     /* round towards +inf */                       \
        rnpqs_rr(W(XG), W(XS))                                              \
        cvzqs_rr(W(XG), W(XG))

#define cvpqs_ld(XG, RM, DP) /* round towards +inf */                       \
        rnpqs_ld(W(XG), W(RM), W(DP))                                       \
        cvzqs_rr(W(XG), W(XG))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XG, XS)     /* round towards -inf */                       \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmqs_ld(XG, RM, DP) /* round towards -inf */                       \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cvmqs_rr(XG, XS)     /* round towards -inf */                       \
        rnmqs_rr(W(XG), W(XS))                                              \
        cvzqs_rr(W(XG), W(XG))

#define cvmqs_ld(XG, RM, DP) /* round towards -inf */                       \
        rnmqs_ld(W(XG), W(RM), W(DP))                                       \
        cvzqs_rr(W(XG), W(XG))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XG, XS)     /* round towards near */                       \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnqs_ld(XG, RM, DP) /* round towards near */                       \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cvnqs_rr(XG, XS)     /* round towards near */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x18))                                    \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define cvnqs_ld(XG, RM, DP) /* round towards near */                       \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        cvnqs_rr(W(XG), W(XG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XG, XS)     /* round towards near */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x18))                                    \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define cvnqn_ld(XG, RM, DP) /* round towards near */                       \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        cvnqn_rr(W(XG), W(XG))

/**************************   packed integer (AVX1)   *************************/

#if (RT_256 < 2)

#define prmqx_rr(XG, XS, IM) /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 3) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define movjx_ld(XG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 0, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movjx_st(XG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 0, 1, 1) EMITB(0x29)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add */

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

#define addjx_ld(XG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 0, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addqx_ld(XG, RM, DP)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addjx_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addjx_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* sub */

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

#define subjx_ld(XG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 0, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subqx_ld(XG, RM, DP)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subjx_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subjx_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shl */

#define shljx_ri(XG, IM)     /* not portable, do not use outside */         \
        VEX(0,       RXB(XG), REN(XG), 0, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlqx_ri(XG, IM)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shljx_ri(W(XG), W(IM))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shljx_ri(W(XG), W(IM))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shljx_ld(XG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 0, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shlqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shljx_ld(W(XG), W(RM), W(DP))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shljx_ld(W(XG), W(RM), W(DP))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shr */

#define shrjx_ri(XG, IM)     /* not portable, do not use outside */         \
        VEX(0,       RXB(XG), REN(XG), 0, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrqx_ri(XG, IM)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrjx_ri(W(XG), W(IM))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrjx_ri(W(XG), W(IM))                                              \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrjx_ld(XG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 0, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrjx_ld(W(XG), W(RM), W(DP))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrjx_ld(W(XG), W(RM), W(DP))                                       \
        movjx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_256 >= 2 */

/* add */

#define addqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addqx_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subqx_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subqx_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlqx_ri(XG, IM)                                                    \
        VEX(0,       RXB(XG), REN(XG), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrqx_ri(XG, IM)                                                    \
        VEX(0,       RXB(XG), REN(XG), 1, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(XG), RXB(RM), REN(XG), 1, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_256 >= 2 */

#define shrqn_ri(XG, IM)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x10), W(IM))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x18), W(IM))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(RM), W(DP))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   helper macros (AVX1)   **************************/

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndqs_ld(XG, RM, DP)                                                \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cvtqs_rr(XG, XS)                                                    \
        rndqs_rr(W(XG), W(XS))                                              \
        cvzqs_rr(W(XG), W(XG))

#define cvtqs_ld(XG, RM, DP)                                                \
        rndqs_ld(W(XG), W(RM), W(DP))                                       \
        cvzqs_rr(W(XG), W(XG))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtqn_rr(XG, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnqn_rr(W(XG), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtqn_ld(XG, RM, DP)                                                \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        cvtqn_rr(W(XG), W(XG))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XG, XS, mode)                                              \
        VEX(RXB(XG), RXB(XS),     0x0, 1, 1, 3) EMITB(0x09)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrqs_rr(XG, XS, mode)                                              \
        rnrqs_rr(W(XG), W(XS), mode)                                        \
        cvzqs_rr(W(XG), W(XG))

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvqx_ld(XG, RM, DP) /* not portable, use conditionally */          \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 2) EMITB(0x2D)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mmvqx_st(XG, RM, DP) /* not portable, use conditionally */          \
    ADR VEX(RXB(XG), RXB(RM),     0x0, 1, 1, 2) EMITB(0x2F)                 \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

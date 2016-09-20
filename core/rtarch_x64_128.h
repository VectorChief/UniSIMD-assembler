/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_128_H
#define RT_RTARCH_X64_128_H

#include "rtarch_x32_128.h"

#if defined (RT_SIMD_CODE)

#if defined (RT_128) && (RT_128 != 0)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_128.h: Implementation of x86_64 fp64 SSE(1,2,4) instructions.
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
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

#if (RT_128 >= 2)

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/**************************   packed generic (SSE2)   *************************/

/* mov */

#define movqx_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movqx_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movqx_st(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andqx_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andqx_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann */

#define annqx_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define annqx_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrqx_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrqx_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orn */

#define ornqx_rr(RG, RM)                                                    \
        notqx_rx(W(RG))                                                     \
        orrqx_rr(W(RG), W(RM))

#define ornqx_ld(RG, RM, DP)                                                \
        notqx_rx(W(RG))                                                     \
        orrqx_ld(W(RG), W(RM), W(DP))

/* xor */

#define xorqx_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorqx_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not */

#define notqx_rx(RM)                                                        \
        annqx_ld(W(RM), Mebp, inf_GPC07)

/**************   packed double precision floating point (SSE2)   *************/

/* neg */

#define negqs_rx(RM)                                                        \
        xorqx_ld(W(RM), Mebp, inf_GPC06_64)

/* add */

#define addqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define divqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqrqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define sqrqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(RG, RM)                                                    \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(RG), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(RG), Mebp, inf_SCR01(0))

#define rcsqs_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(RG, RM)                                                    \
        sqrqs_rr(W(RG), W(RM))                                              \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(RG), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(RG), Mebp, inf_SCR01(0))

#define rssqs_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define minqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define maxqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/**************************   packed integer (SSE2)   *************************/

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

#define fpurz_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#if (RT_128 < 4)

#define rnzqs_rr(RG, RM)     /* round towards zero */                       \
        cvzqs_rr(W(RG), W(RM))                                              \
        cvnqn_rr(W(RG), W(RG))

#define rnzqs_ld(RG, RM, DP) /* round towards zero */                       \
        cvzqs_ld(W(RG), W(RM), W(DP))                                       \
        cvnqn_rr(W(RG), W(RG))

#else /* RT_128 >= 4 */

#define rnzqs_rr(RG, RM)     /* round towards zero */                       \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzqs_ld(RG, RM, DP) /* round towards zero */                       \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x03))

#endif /* RT_128 >= 4 */

#define cvzqs_rr(RG, RM)     /* round towards zero */                       \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x08))                                    \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvzqs_ld(RG, RM, DP) /* round towards zero */                       \
        movqx_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#if (RT_128 < 4)

#define rnpqs_rr(RG, RM)     /* round towards +inf */                       \
        cvpqs_rr(W(RG), W(RM))                                              \
        cvnqn_rr(W(RG), W(RG))

#define rnpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        cvpqs_ld(W(RG), W(RM), W(DP))                                       \
        cvnqn_rr(W(RG), W(RG))

#define cvpqs_rr(RG, RM)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnqs_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnqs_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

#else /* RT_128 >= 4 */

#define rnpqs_rr(RG, RM)     /* round towards +inf */                       \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpqs_ld(RG, RM, DP) /* round towards +inf */                       \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cvpqs_rr(RG, RM)     /* round towards +inf */                       \
        rnpqs_rr(W(RG), W(RM))                                              \
        cvzqs_rr(W(RG), W(RG))

#define cvpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        rnpqs_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

#endif /* RT_128 >= 4 */

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#if (RT_128 < 4)

#define rnmqs_rr(RG, RM)     /* round towards -inf */                       \
        cvmqs_rr(W(RG), W(RM))                                              \
        cvnqn_rr(W(RG), W(RG))

#define rnmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        cvmqs_ld(W(RG), W(RM), W(DP))                                       \
        cvnqn_rr(W(RG), W(RG))

#define cvmqs_rr(RG, RM)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnqs_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnqs_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

#else /* RT_128 >= 4 */

#define rnmqs_rr(RG, RM)     /* round towards -inf */                       \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmqs_ld(RG, RM, DP) /* round towards -inf */                       \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cvmqs_rr(RG, RM)     /* round towards -inf */                       \
        rnmqs_rr(W(RG), W(RM))                                              \
        cvzqs_rr(W(RG), W(RG))

#define cvmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        rnmqs_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

#endif /* RT_128 >= 4 */

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR02(4))

#if (RT_128 < 4)

#define rnnqs_rr(RG, RM)     /* round towards near */                       \
        cvnqs_rr(W(RG), W(RM))                                              \
        cvnqn_rr(W(RG), W(RG))

#define rnnqs_ld(RG, RM, DP) /* round towards near */                       \
        cvnqs_ld(W(RG), W(RM), W(DP))                                       \
        cvnqn_rr(W(RG), W(RG))

#else /* RT_128 >= 4 */

#define rnnqs_rr(RG, RM)     /* round towards near */                       \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnqs_ld(RG, RM, DP) /* round towards near */                       \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#endif /* RT_128 >= 4 */

#define cvnqs_rr(RG, RM)     /* round towards near */                       \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x08))                                    \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnqs_ld(RG, RM, DP) /* round towards near */                       \
        movqx_ld(W(RG), W(RM), W(DP))                                       \
        cvnqs_rr(W(RG), W(RG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(RG, RM)     /* round towards near */                       \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuzn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x08))                                    \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnqn_ld(RG, RM, DP) /* round towards near */                       \
        movqx_ld(W(RG), W(RM), W(DP))                                       \
        cvnqn_rr(W(RG), W(RG))

/* add */

#define addqx_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addqx_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subqx_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subqx_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlqx_ri(RM, IM)                                                    \
    ESC REX(0,       RXB(RM)) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xF3)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrqx_ri(RM, IM)                                                    \
    ESC REX(0,       RXB(RM)) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xD3)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrqn_ri(RM, IM)                                                    \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movqx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrqn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(RM), W(DP))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

/**************************   helper macros (SSE2)   **************************/

#if (RT_128 < 4)

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(RG, RM)                                                    \
        cvtqs_rr(W(RG), W(RM))                                              \
        cvnqn_rr(W(RG), W(RG))

#define rndqs_ld(RG, RM, DP)                                                \
        cvtqs_ld(W(RG), W(RM), W(DP))                                       \
        cvnqn_rr(W(RG), W(RG))

#define cvtqs_rr(RG, RM)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnqs_rr(W(RG), W(RM))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtqs_ld(RG, RM, DP)                                                \
        movqx_ld(W(RG), W(RM), W(DP))                                       \
        cvtqs_rr(W(RG), W(RG))

#else /* RT_128 >= 4 */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(RG, RM)                                                    \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndqs_ld(RG, RM, DP)                                                \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cvtqs_rr(RG, RM)                                                    \
        rndqs_rr(W(RG), W(RM))                                              \
        cvzqs_rr(W(RG), W(RG))

#define cvtqs_ld(RG, RM, DP)                                                \
        rndqs_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

#endif /* RT_128 >= 4 */

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtqn_rr(RG, RM)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnqn_rr(W(RG), W(RM))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtqn_ld(RG, RM, DP)                                                \
        movqx_ld(W(RG), W(RM), W(DP))                                       \
        cvtqn_rr(W(RG), W(RG))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_128 < 4)

#define rnrqs_rr(RG, RM, mode)                                              \
        cvrqs_rr(W(RG), W(RM), mode)                                        \
        cvnqn_rr(W(RG), W(RG))

#define cvrqs_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtqs_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_128 >= 4 */

#define rnrqs_rr(RG, RM, mode)                                              \
    ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrqs_rr(RG, RM, mode)                                              \
        rnrqs_rr(W(RG), W(RM), mode)                                        \
        cvzqs_rr(W(RG), W(RG))

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvqx_ld(RG, RM, DP) /* not portable, use conditionally */          \
ADR ESC REX(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0x38) EMITB(0x15)           \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 4 */

#endif /* RT_128 >= 2 */

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

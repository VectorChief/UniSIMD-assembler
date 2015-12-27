/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_256_H
#define RT_RTARCH_X32_256_H

#include "rtarch_x32.h"

#define RT_SIMD_WIDTH       8
#define RT_SIMD_ALIGN       32
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v

#if defined (RT_SIMD_CODE)

#if defined (RT_256) && (RT_256 != 0)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_256.h: Implementation of x86_64:x32 AVX2-256 instructions.
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
 * The cmdp*_** instructions are intended for SPMD programming model
 * and can potentially be configured per target to work with 32-bit/64-bit
 * data-elements (integers/pointers, floating point).
 * In this model data-paths are fixed-width, core and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* 3-byte VEX prefix with full customization */
#define VEX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0xC4)                                                         \
        EMITB((1 - (rxg)) << 7 | 1 << 6 | (1 - (rxm)) << 5 | (aux))         \
        EMITB((len) << 2 | (0x0F - (ren)) << 3 | (pfx))

/* selector for full register (3rd operand, 4-bits-wide) */
#define REN(reg, mod, sib)  reg

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x03, EMPTY
#define Xmm1    0x01, 0x03, EMPTY
#define Xmm2    0x02, 0x03, EMPTY
#define Xmm3    0x03, 0x03, EMPTY
#define Xmm4    0x04, 0x03, EMPTY
#define Xmm5    0x05, 0x03, EMPTY
#define Xmm6    0x06, 0x03, EMPTY
#define Xmm7    0x07, 0x03, EMPTY
#define Xmm8    0x08, 0x03, EMPTY
#define Xmm9    0x09, 0x03, EMPTY
#define XmmA    0x0A, 0x03, EMPTY
#define XmmB    0x0B, 0x03, EMPTY
#define XmmC    0x0C, 0x03, EMPTY
#define XmmD    0x0D, 0x03, EMPTY
#define XmmE    0x0E, 0x03, EMPTY
#define XmmF    0x0F, 0x03, EMPTY

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movpx_st(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* RG is a core reg, DP is SIMD-aligned */     \
    ADR REX(RXB(RG), RXB(RM)) EMITB(0x8D)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann */

#define annpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define annpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/**************   packed single precision floating point (AVX1)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define divps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define sqrps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x53)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define rcsps_rr(RG, RM) /* destroys RM */                                  \
        mulps_rr(W(RM), W(RG))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        addps_rr(W(RG), W(RG))                                              \
        subps_rr(W(RG), W(RM))

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq */

#define rseps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x52)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define rssps_rr(RG, RM) /* destroys RM */                                  \
        mulps_rr(W(RM), W(RG))                                              \
        mulps_rr(W(RM), W(RG))                                              \
        subps_ld(W(RM), Mebp, inf_GPC03)                                    \
        mulps_ld(W(RM), Mebp, inf_GPC02)                                    \
        mulps_rr(W(RG), W(RM))

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define minps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define maxps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/* cvz
 * rounding mode is encoded directly    (can be used in FCTRL blocks) */

#define cvzps_rr(RG, RM)     /* round towards zero */                       \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvzps_ld(RG, RM, DP) /* round towards zero */                       \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvt
 * rounding mode comes from fp control register (set in FCTRL blocks) */

#define cvtps_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvtps_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define cvtpn_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvtpn_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvn
 * rounding mode is encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round to nearest */                         \
        cvtpn_rr(W(RG), W(RM))

#define cvnpn_ld(RG, RM, DP) /* round to nearest */                         \
        cvtpn_ld(W(RG), W(RM), W(DP))

/**************************   packed integer (AVX1)   *************************/

#if (RT_256 < 2)

#define prmpx_rr(RG, RM, IM) /* not portable, do not use outside */         \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 3) EMITB(0x06)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define movlx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movlx_st(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM),     0x0, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add */

#define addlx_rr(RG, RM)     /* not portable, do not use outside */         \
        VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xFE)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        addlx_rr(W(RG), W(RM))                                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        addlx_rr(W(RG), W(RM))                                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define addlx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xFE)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x00))                              \
        addlx_ld(W(RG), Mebp, inf_SCR02(0x00))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        addlx_ld(W(RG), Mebp, inf_SCR02(0x10))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* sub */

#define sublx_rr(RG, RM)     /* not portable, do not use outside */         \
        VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xFA)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subpx_rr(RG, RM)                                                    \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        sublx_rr(W(RG), W(RM))                                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        sublx_rr(W(RG), W(RM))                                              \
        prmpx_rr(W(RM), W(RM), IB(1))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define sublx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xFA)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movpx_ld(W(RG), W(RM), W(DP))                                       \
        movpx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x00))                              \
        sublx_ld(W(RG), Mebp, inf_SCR02(0x00))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        sublx_ld(W(RG), Mebp, inf_SCR02(0x10))                              \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shl */

#define shllx_ri(RM, IM)     /* not portable, do not use outside */         \
        VEX(0,       RXB(RM), REN(RM), 0, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shllx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RM), Mebp, inf_SCR01(0x10))                              \
        shllx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shllx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shlpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        shllx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        shllx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/* shr */

#define shrlx_ri(RM, IM)     /* not portable, do not use outside */         \
        VEX(0,       RXB(RM), REN(RM), 0, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrpx_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrlx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RM), Mebp, inf_SCR01(0x10))                              \
        shrlx_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrlx_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xD2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrpx_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        shrlx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        shrlx_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

#define shrln_ri(RM, IM)     /* not portable, do not use outside */         \
        VEX(0,       RXB(RM), REN(RM), 0, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrpn_ri(RM, IM)                                                    \
        movpx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrln_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RM), Mebp, inf_SCR01(0x10))                              \
        shrln_ri(W(RM), W(IM))                                              \
        movlx_st(W(RM), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrln_ld(RG, RM, DP) /* not portable, do not use outside */         \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 0, 1, 1) EMITB(0xE2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrpn_ld(RG, RM, DP)                                                \
        movpx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        shrln_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(RG), Mebp, inf_SCR01(0x10))                              \
        shrln_ld(W(RG), W(RM), W(DP))                                       \
        movlx_st(W(RG), Mebp, inf_SCR01(0x10))                              \
        movpx_ld(W(RG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_256 >= 2 */

/* add */

#define addpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subpx_rr(RG, RM)                                                    \
        VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        VEX(0,       RXB(RM), REN(RM), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        VEX(0,       RXB(RM), REN(RM), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrpx_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xD2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrpn_ri(RM, IM)                                                    \
        VEX(0,       RXB(RM), REN(RM), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrpn_ld(RG, RM, DP)                                                \
    ADR VEX(RXB(RG), RXB(RM), REN(RG), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_256 >= 2 */

/**************************   helper macros (AVX1)   **************************/

/* simd mask */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0xFF    /*  all satisfy the condition */

#define movsn_rr(RG, RM) /* not portable, do not use outside */             \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 0, 1) EMITB(0x50)                 \
        MRM(REG(RG), MOD(RM), REG(RM))

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        movsn_rr(Reax, W(RG))                                               \
        cmpxx_ri(Reax, IH(RT_SIMD_MASK_##mask))                             \
        jeqxx_lb(lb)

/* simd mode
 * set in FCTRL blocks (cannot be nested) */

#define RT_SIMD_MODE_ROUNDN     0x00    /* round to nearest */
#define RT_SIMD_MODE_ROUNDM     0x01    /* round towards minus infinity */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards plus  infinity */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
    ADR VEX(0,       RXB(RM),     0x0, 0, 0, 1) EMITB(0xAE)                 \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
    ADR VEX(0,       RXB(RM),     0x0, 0, 0, 1) EMITB(0xAE)                 \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define FCTRL_ENTER(mode) /* destroys Reax, assumes default upon entry */   \
        movxx_mi(Mebp, inf_FCTRL, IH(RT_SIMD_MODE_##mode << 13 | 0x1F80))   \
        mxcsr_ld(Mebp, inf_FCTRL)                                           \

#define FCTRL_LEAVE(mode) /* destroys Reax, resumes default upon entry */   \
        movxx_mi(Mebp, inf_FCTRL, IH(RT_SIMD_MODE_ROUNDN << 13 | 0x1F80))   \
        mxcsr_ld(Mebp, inf_FCTRL)

/* cvr
 * rounding mode is encoded directly (cannot be used in FCTRL blocks) */

#define rndps_rr(RG, RM, mode) /* not portable, do not use outside */       \
        VEX(RXB(RG), RXB(RM),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode))

#define cvrps_rr(RG, RM, mode)                                              \
        rndps_rr(W(RG), W(RM), mode)                                        \
        cvtps_rr(W(RG), W(RG))

/* mmv */

#define mmvpx_ld(RG, RM, DP, RN) /* not portable, use conditionally */      \
    ADR VEX(RXB(RG), RXB(RM), REN(RN), 1, 1, 2) EMITB(0x2C)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mmvpx_st(RG, RM, DP, RN) /* not portable, use conditionally */      \
    ADR VEX(RXB(RG), RXB(RM), REN(RN), 1, 1, 2) EMITB(0x2E)                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

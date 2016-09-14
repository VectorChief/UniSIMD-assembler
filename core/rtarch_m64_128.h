/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M64_128_H
#define RT_RTARCH_M64_128_H

#include "rtarch_m32_128.h"

#if defined (RT_SIMD_CODE)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m64_128.h: Implementation of MIPS fp64 MSA instructions.
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

/******************************************************************************/
/**********************************   MSA   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov */

#define movqx_rr(RG, RM)                                                    \
        EMITW(0x78BE0019 | MXM(REG(RG), REG(RM), 0x00))

#define movqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

#define movqx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000027 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

/* and */

#define andqx_rr(RG, RM)                                                    \
        EMITW(0x7800001E | MXM(REG(RG), REG(RG), REG(RM)))

#define andqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7800001E | MXM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annqx_rr(RG, RM)                                                    \
        EMITW(0x78C0001E | MXM(REG(RG), REG(RM), TmmZ))

#define annqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78C0001E | MXM(REG(RG), Tmm1,    TmmZ))

/* orr */

#define orrqx_rr(RG, RM)                                                    \
        EMITW(0x7820001E | MXM(REG(RG), REG(RG), REG(RM)))

#define orrqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7820001E | MXM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorqx_rr(RG, RM)                                                    \
        EMITW(0x7860001E | MXM(REG(RG), REG(RG), REG(RM)))

#define xorqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7860001E | MXM(REG(RG), REG(RG), Tmm1))

/**************   packed double precision floating point (SIMD)   *************/

/* add */

#define addqs_rr(RG, RM)                                                    \
        EMITW(0x7820001B | MXM(REG(RG), REG(RG), REG(RM)))

#define addqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7820001B | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subqs_rr(RG, RM)                                                    \
        EMITW(0x7860001B | MXM(REG(RG), REG(RG), REG(RM)))

#define subqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7860001B | MXM(REG(RG), REG(RG), Tmm1))

/* mul */

#define mulqs_rr(RG, RM)                                                    \
        EMITW(0x78A0001B | MXM(REG(RG), REG(RG), REG(RM)))

#define mulqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78A0001B | MXM(REG(RG), REG(RG), Tmm1))

/* div */

#define divqs_rr(RG, RM)                                                    \
        EMITW(0x78E0001B | MXM(REG(RG), REG(RG), REG(RM)))

#define divqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78E0001B | MXM(REG(RG), REG(RG), Tmm1))

/* sqr */

#define sqrqs_rr(RG, RM)                                                    \
        EMITW(0x7B27001E | MXM(REG(RG), REG(RM), 0x00))

#define sqrqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B27001E | MXM(REG(RG), Tmm1,    0x00))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(RG, RM)                                                    \
        EMITW(0x7B2B001E | MXM(REG(RG), REG(RM), 0x00))

#define rcsqs_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(RG, RM)                                                    \
        EMITW(0x7B29001E | MXM(REG(RG), REG(RM), 0x00))

#define rssqs_rr(RG, RM) /* destroys RM */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minqs_rr(RG, RM)                                                    \
        EMITW(0x7B20001B | MXM(REG(RG), REG(RG), REG(RM)))

#define minqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B20001B | MXM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxqs_rr(RG, RM)                                                    \
        EMITW(0x7BA0001B | MXM(REG(RG), REG(RG), REG(RM)))

#define maxqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7BA0001B | MXM(REG(RG), REG(RG), Tmm1))

/* cmp */

#define ceqqs_rr(RG, RM)                                                    \
        EMITW(0x78A0001A | MXM(REG(RG), REG(RG), REG(RM)))

#define ceqqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78A0001A | MXM(REG(RG), REG(RG), Tmm1))

#define cneqs_rr(RG, RM)                                                    \
        EMITW(0x78E0001C | MXM(REG(RG), REG(RG), REG(RM)))

#define cneqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78E0001C | MXM(REG(RG), REG(RG), Tmm1))

#define cltqs_rr(RG, RM)                                                    \
        EMITW(0x7920001A | MXM(REG(RG), REG(RG), REG(RM)))

#define cltqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7920001A | MXM(REG(RG), REG(RG), Tmm1))

#define cleqs_rr(RG, RM)                                                    \
        EMITW(0x79A0001A | MXM(REG(RG), REG(RG), REG(RM)))

#define cleqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x79A0001A | MXM(REG(RG), REG(RG), Tmm1))

#define cgtqs_rr(RG, RM)                                                    \
        EMITW(0x7920001A | MXM(REG(RG), REG(RM), REG(RG)))

#define cgtqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7920001A | MXM(REG(RG), Tmm1,    REG(RG)))

#define cgeqs_rr(RG, RM)                                                    \
        EMITW(0x79A0001A | MXM(REG(RG), REG(RM), REG(RG)))

#define cgeqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x79A0001A | MXM(REG(RG), Tmm1,    REG(RG)))

/**************************   packed integer (SIMD)   *************************/

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(RG, RM)     /* round towards zero */                       \
        cvzqs_rr(W(RG), W(RM))                                              \
        cvnqn_rr(W(RG), W(RG))

#define rnzqs_ld(RG, RM, DP) /* round towards zero */                       \
        cvzqs_ld(W(RG), W(RM), W(DP))                                       \
        cvnqn_rr(W(RG), W(RG))

#define cvzqs_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0x7B23001E | MXM(REG(RG), REG(RM), 0x00))

#define cvzqs_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B23001E | MXM(REG(RG), Tmm1,    0x00))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(RG, RM)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndqs_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rnpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndqs_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cvpqs_rr(RG, RM)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtqs_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtqs_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(RG, RM)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndqs_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rnmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndqs_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cvmqs_rr(RG, RM)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtqs_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtqs_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(RG, RM)     /* round towards near */                       \
        rndqs_rr(W(RG), W(RM))

#define rnnqs_ld(RG, RM, DP) /* round towards near */                       \
        rndqs_ld(W(RG), W(RM), W(DP))

#define cvnqs_rr(RG, RM)     /* round towards near */                       \
        cvtqs_rr(W(RG), W(RM))

#define cvnqs_ld(RG, RM, DP) /* round towards near */                       \
        cvtqs_ld(W(RG), W(RM), W(DP))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(RG, RM)     /* round towards near */                       \
        cvtqn_rr(W(RG), W(RM))

#define cvnqn_ld(RG, RM, DP) /* round towards near */                       \
        cvtqn_ld(W(RG), W(RM), W(DP))

/* add */

#define addqx_rr(RG, RM)                                                    \
        EMITW(0x7860000E | MXM(REG(RG), REG(RG), REG(RM)))

#define addqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7860000E | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subqx_rr(RG, RM)                                                    \
        EMITW(0x78E0000E | MXM(REG(RG), REG(RG), REG(RM)))

#define subqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78E0000E | MXM(REG(RG), REG(RG), Tmm1))

/* shl */

#define shlqx_ri(RM, IM)                                                    \
        EMITW(0x78000009 | MXM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x3F & VAL(IM)) << 16)

#define shlqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7B03001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x7860000D | MXM(REG(RG), REG(RG), Tmm1))

/* shr */

#define shrqx_ri(RM, IM)                                                    \
        EMITW(0x79000009 | MXM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x3F & VAL(IM)) << 16)

#define shrqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7B03001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x7960000D | MXM(REG(RG), REG(RG), Tmm1))

#define shrqn_ri(RM, IM)                                                    \
        EMITW(0x78800009 | MXM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x3F & VAL(IM)) << 16)

#define shrqn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7B03001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x78E0000D | MXM(REG(RG), REG(RG), Tmm1))

/**************************   helper macros (SIMD)   **************************/

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(RG, RM)                                                    \
        EMITW(0x7B2D001E | MXM(REG(RG), REG(RM), 0x00))

#define rndqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B2D001E | MXM(REG(RG), Tmm1,    0x00))

#define cvtqs_rr(RG, RM)                                                    \
        EMITW(0x7B39001E | MXM(REG(RG), REG(RM), 0x00))

#define cvtqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B39001E | MXM(REG(RG), Tmm1,    0x00))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtqn_rr(RG, RM)                                                    \
        EMITW(0x7B3D001E | MXM(REG(RG), REG(RM), 0x00))

#define cvtqn_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B3D001E | MXM(REG(RG), Tmm1,    0x00))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndqs_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#define cvrqs_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtqs_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M64_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

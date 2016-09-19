/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P64_128_H
#define RT_RTARCH_P64_128_H

#include "rtarch_p32_128.h"

#if defined (RT_SIMD_CODE)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p64_128.h: Implementation of Power fp64 VMX/VSX instructions.
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

#undef  movqx_ld

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov */

#define movqx_rr(RG, RM)                                                    \
        EMITW(0xF0000497 | MXM(REG(RG), REG(RM), REG(RM)))

#define movqx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(REG(RG), Teax & (MOD(RM) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movqx_st(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000799 | MXM(REG(RG), Teax & (MOD(RM) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* and */

#define andqx_rr(RG, RM)                                                    \
        EMITW(0xF0000417 | MXM(REG(RG), REG(RG), REG(RM)))

#define andqx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* ann */

#define annqx_rr(RG, RM)                                                    \
        EMITW(0xF0000457 | MXM(REG(RG), REG(RM), REG(RG)))

#define annqx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(RG), Tmm1,    REG(RG)))/* ^ == -1 if true */

/* orr */

#define orrqx_rr(RG, RM)                                                    \
        EMITW(0xF0000497 | MXM(REG(RG), REG(RG), REG(RM)))

#define orrqx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* xor */

#define xorqx_rr(RG, RM)                                                    \
        EMITW(0xF00004D7 | MXM(REG(RG), REG(RG), REG(RM)))

#define xorqx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/**************   packed double precision floating point (SIMD)   *************/

/* add */

#define addqs_rr(RG, RM)                                                    \
        EMITW(0xF0000307 | MXM(REG(RG), REG(RG), REG(RM)))

#define addqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000307 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* sub */

#define subqs_rr(RG, RM)                                                    \
        EMITW(0xF0000347 | MXM(REG(RG), REG(RG), REG(RM)))

#define subqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000347 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* mul */

#define mulqs_rr(RG, RM)                                                    \
        EMITW(0xF0000387 | MXM(REG(RG), REG(RG), REG(RM)))

#define mulqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000387 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* div */

#define divqs_rr(RG, RM)                                                    \
        EMITW(0xF00003C7 | MXM(REG(RG), REG(RG), REG(RM)))

#define divqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003C7 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* sqr */

#define sqrqs_rr(RG, RM)                                                    \
        EMITW(0xF000032F | MXM(REG(RG), 0x00,    REG(RM)))

#define sqrqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000032F | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(RG, RM)                                                    \
        EMITW(0xF000036B | MXM(REG(RG), 0x00,    REG(RM)))

#define rcsqs_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF00007CF | MXM(REG(RM), REG(RG), TmmX))                     \
        EMITW(0xF000030F | MXM(REG(RG), REG(RG), REG(RM)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(RG, RM)                                                    \
        EMITW(0xF000032B | MXM(REG(RG), 0x00,    REG(RM)))

#define rssqs_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF0000387 | MXM(TmmD,    REG(RG), REG(RG)))                  \
        EMITW(0xF0000387 | MXM(TmmC,    REG(RG), TmmY))                     \
        EMITW(0xF00007CF | MXM(TmmD,    REG(RM), TmmX))                     \
        EMITW(0xF000078F | MXM(REG(RG), TmmD,    TmmC))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minqs_rr(RG, RM)                                                    \
        EMITW(0xF0000747 | MXM(REG(RG), REG(RG), REG(RM)))

#define minqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000747 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* max */

#define maxqs_rr(RG, RM)                                                    \
        EMITW(0xF0000707 | MXM(REG(RG), REG(RG), REG(RM)))

#define maxqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000707 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* cmp */

#define ceqqs_rr(RG, RM)                                                    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), REG(RM)))

#define ceqqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#define cneqs_rr(RG, RM)                                                    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF0000517 | MXM(REG(RG), REG(RG), REG(RG)))

#define cneqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(RG), REG(RG), REG(RG)))

#define cltqs_rr(RG, RM)                                                    \
        EMITW(0xF000035F | MXM(REG(RG), REG(RM), REG(RG)))

#define cltqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(RG), Tmm1,    REG(RG)))/* ^ == -1 if true */

#define cleqs_rr(RG, RM)                                                    \
        EMITW(0xF000039F | MXM(REG(RG), REG(RM), REG(RG)))

#define cleqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(RG), Tmm1,    REG(RG)))/* ^ == -1 if true */

#define cgtqs_rr(RG, RM)                                                    \
        EMITW(0xF000035F | MXM(REG(RG), REG(RG), REG(RM)))

#define cgtqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#define cgeqs_rr(RG, RM)                                                    \
        EMITW(0xF000039F | MXM(REG(RG), REG(RG), REG(RM)))

#define cgeqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0xF0000367 | MXM(REG(RG), 0x00,    REG(RM)))

#define rnzqs_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000367 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvzqs_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0xF0000763 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvzqs_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000763 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(RG, RM)     /* round towards +inf */                       \
        EMITW(0xF00003A7 | MXM(REG(RG), 0x00,    REG(RM)))

#define rnpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003A7 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvpqs_rr(RG, RM)     /* round towards +inf */                       \
        rnpqs_rr(W(RG), W(RM))                                              \
        cvzqs_rr(W(RG), W(RG))

#define cvpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        rnpqs_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(RG, RM)     /* round towards -inf */                       \
        EMITW(0xF00003E7 | MXM(REG(RG), 0x00,    REG(RM)))

#define rnmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003E7 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvmqs_rr(RG, RM)     /* round towards -inf */                       \
        rnmqs_rr(W(RG), W(RM))                                              \
        cvzqs_rr(W(RG), W(RG))

#define cvmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        rnmqs_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(RG, RM)     /* round towards near */                       \
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    REG(RM)))

#define rnnqs_ld(RG, RM, DP) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvnqs_rr(RG, RM)     /* round towards near */                       \
        rnnqs_rr(W(RG), W(RM))                                              \
        cvzqs_rr(W(RG), W(RG))

#define cvnqs_ld(RG, RM, DP) /* round towards near */                       \
        rnnqs_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(RG, RM)     /* round towards near */                       \
        cvtqn_rr(W(RG), W(RM))

#define cvnqn_ld(RG, RM, DP) /* round towards near */                       \
        cvtqn_ld(W(RG), W(RM), W(DP))

/**************************   packed integer (SIMD)   *************************/

#if RT_SIMD_COMPAT_I64 != 0

/* add */

#define addqx_rr(RG, RM)                                                    \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(RM), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

#define addqx_ld(RG, RM, DP)                                                \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(RG), W(RM), W(DP))                                       \
        movqx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

/* sub */

#define subqx_rr(RG, RM)                                                    \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(RM), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

#define subqx_ld(RG, RM, DP)                                                \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(RG), W(RM), W(DP))                                       \
        movqx_st(W(RG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

/* shl */

#define shlqx_ri(RM, IM)                                                    \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shlzx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movqx_ld(W(RM), Mebp, inf_SCR01(0))

#define shlqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(RM), W(DP))                                       \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

/* shr */

#define shrqx_ri(RM, IM)                                                    \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrzx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movqx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(RM), W(DP))                                       \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

#define shrqn_ri(RM, IM)                                                    \
        movqx_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movqx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrqn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(RM), W(DP))                                       \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(RG), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_I64 */

/* add */

#define addqx_rr(RG, RM)                                                    \
        EMITW(0x100000C0 | MXM(REG(RG), REG(RG), REG(RM)))

#define addqx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* sub */

#define subqx_rr(RG, RM)                                                    \
        EMITW(0x100004C0 | MXM(REG(RG), REG(RG), REG(RM)))

#define subqx_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* shl */

#define shlqx_ri(RM, IM)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        shlqx_ld(W(RM), Mebp, inf_SCR01(0))

#define shlqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

/* shr */

#define shrqx_ri(RM, IM)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        shrqx_ld(W(RM), Mebp, inf_SCR01(0))

#define shrqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#define shrqn_ri(RM, IM)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        shrqn_ld(W(RM), Mebp, inf_SCR01(0))

#define shrqn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(REG(RG), REG(RG), Tmm1))/* ^ == -1 if true */

#endif /* RT_SIMD_COMPAT_I64 */

/**************************   helper macros (SIMD)   **************************/

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(RG, RM)                                                    \
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    REG(RM)))

#define rndqs_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvtqs_rr(RG, RM)                                                    \
        rndqs_rr(W(RG), W(RM))                                              \
        cvzqs_rr(W(RG), W(RG))

#define cvtqs_ld(RG, RM, DP)                                                \
        rndqs_ld(W(RG), W(RM), W(DP))                                       \
        cvzqs_rr(W(RG), W(RG))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtqn_rr(RG, RM)                                                    \
        EMITW(0xF00007E3 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvtqn_ld(RG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00007E3 | MXM(REG(RG), 0x00,    Tmm1))/* ^ == -1 if true */

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
        rnrqs_rr(W(RG), W(RM), mode)                                        \
        cvzqs_rr(W(RG), W(RG))

#endif /* RT_128 >= 2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P64_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

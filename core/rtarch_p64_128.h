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

#if (RT_128 >= 2)

#undef  movqx_ld

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov */

#define movqx_rr(XG, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XS), REG(XS)))

#define movqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(REG(XG), Teax & (MOD(RM) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movqx_st(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000799 | MXM(REG(XG), Teax & (MOD(RM) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* and */

#define andqx_rr(XG, XS)                                                    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), REG(XS)))

#define andqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* ann (~XG & XS) */

#define annqx_rr(XG, XS)                                                    \
        EMITW(0xF0000457 | MXM(REG(XG), REG(XS), REG(XG)))

#define annqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

/* orr */

#define orrqx_rr(XG, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* orn (~XG | XS) */

#define ornqx_rr(XG, XS)                                                    \
        EMITW(0xF0000557 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

/* xor */

#define xorqx_rr(XG, XS)                                                    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* not */

#define notqx_rx(XG)                                                        \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

/**************   packed double precision floating point (SIMD)   *************/

/* neg */

#define negqs_rx(XG)                                                        \
        EMITW(0xF00007E7 | MXM(REG(XG), 0x00,    REG(XG)))

/* add */

#define addqs_rr(XG, XS)                                                    \
        EMITW(0xF0000307 | MXM(REG(XG), REG(XG), REG(XS)))

#define addqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000307 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* sub */

#define subqs_rr(XG, XS)                                                    \
        EMITW(0xF0000347 | MXM(REG(XG), REG(XG), REG(XS)))

#define subqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000347 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* mul */

#define mulqs_rr(XG, XS)                                                    \
        EMITW(0xF0000387 | MXM(REG(XG), REG(XG), REG(XS)))

#define mulqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000387 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* div */

#define divqs_rr(XG, XS)                                                    \
        EMITW(0xF00003C7 | MXM(REG(XG), REG(XG), REG(XS)))

#define divqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003C7 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* sqr */

#define sqrqs_rr(XG, XS)                                                    \
        EMITW(0xF000032F | MXM(REG(XG), 0x00,    REG(XS)))

#define sqrqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000032F | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(XG, XS)                                                    \
        EMITW(0xF000036B | MXM(REG(XG), 0x00,    REG(XS)))

#define rcsqs_rr(XG, XS) /* destroys RM */                                  \
        EMITW(0xF00007CF | MXM(REG(XS), REG(XG), TmmX))                     \
        EMITW(0xF000030F | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(XG, XS)                                                    \
        EMITW(0xF000032B | MXM(REG(XG), 0x00,    REG(XS)))

#define rssqs_rr(XG, XS) /* destroys RM */                                  \
        EMITW(0xF0000387 | MXM(TmmD,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000387 | MXM(TmmC,    REG(XG), TmmY))                     \
        EMITW(0xF00007CF | MXM(TmmD,    REG(XS), TmmX))                     \
        EMITW(0xF000078F | MXM(REG(XG), TmmD,    TmmC))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minqs_rr(XG, XS)                                                    \
        EMITW(0xF0000747 | MXM(REG(XG), REG(XG), REG(XS)))

#define minqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000747 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* max */

#define maxqs_rr(XG, XS)                                                    \
        EMITW(0xF0000707 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000707 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* cmp */

#define ceqqs_rr(XG, XS)                                                    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#define cneqs_rr(XG, XS)                                                    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cneqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cltqs_rr(XG, XS)                                                    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XS), REG(XG)))

#define cltqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#define cleqs_rr(XG, XS)                                                    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XS), REG(XG)))

#define cleqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#define cgtqs_rr(XG, XS)                                                    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#define cgeqs_rr(XG, XS)                                                    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgeqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XG, XS)     /* round towards zero */                       \
        EMITW(0xF0000367 | MXM(REG(XG), 0x00,    REG(XS)))

#define rnzqs_ld(XG, RM, DP) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000367 | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvzqs_rr(XG, XS)     /* round towards zero */                       \
        EMITW(0xF0000763 | MXM(REG(XG), 0x00,    REG(XS)))

#define cvzqs_ld(XG, RM, DP) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF0000763 | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XG, XS)     /* round towards +inf */                       \
        EMITW(0xF00003A7 | MXM(REG(XG), 0x00,    REG(XS)))

#define rnpqs_ld(XG, RM, DP) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003A7 | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

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
        EMITW(0xF00003E7 | MXM(REG(XG), 0x00,    REG(XS)))

#define rnmqs_ld(XG, RM, DP) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003E7 | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

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
        EMITW(0xF00003AF | MXM(REG(XG), 0x00,    REG(XS)))

#define rnnqs_ld(XG, RM, DP) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvnqs_rr(XG, XS)     /* round towards near */                       \
        rnnqs_rr(W(XG), W(XS))                                              \
        cvzqs_rr(W(XG), W(XG))

#define cvnqs_ld(XG, RM, DP) /* round towards near */                       \
        rnnqs_ld(W(XG), W(RM), W(DP))                                       \
        cvzqs_rr(W(XG), W(XG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XG, XS)     /* round towards near */                       \
        cvtqn_rr(W(XG), W(XS))

#define cvnqn_ld(XG, RM, DP) /* round towards near */                       \
        cvtqn_ld(W(XG), W(RM), W(DP))

/**************************   packed integer (SIMD)   *************************/

#if RT_SIMD_COMPAT_I64 != 0

/* add */

#define addqx_rr(XG, XS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define addqx_ld(XG, RM, DP)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* sub */

#define subqx_rr(XG, XS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define subqx_ld(XG, RM, DP)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(RM), W(DP))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shl */

#define shlqx_ri(XG, IM)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlzx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shlqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(RM), W(DP))                                       \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shr */

#define shrqx_ri(XG, IM)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(RM), W(DP))                                       \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ri(XG, IM)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(RM), W(DP))                                       \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_I64 */

/* add */

#define addqx_rr(XG, XS)                                                    \
        EMITW(0x100000C0 | MXM(REG(XG), REG(XG), REG(XS)))

#define addqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* sub */

#define subqx_rr(XG, XS)                                                    \
        EMITW(0x100004C0 | MXM(REG(XG), REG(XG), REG(XS)))

#define subqx_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* shl */

#define shlqx_ri(XG, IM)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        shlqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shlqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* shr */

#define shrqx_ri(XG, IM)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        shrqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqx_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#define shrqn_ri(XG, IM)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        shrqn_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000299 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#endif /* RT_SIMD_COMPAT_I64 */

/**************************   helper macros (SIMD)   **************************/

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XG, XS)                                                    \
        EMITW(0xF00003AF | MXM(REG(XG), 0x00,    REG(XS)))

#define rndqs_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

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
        EMITW(0xF00007E3 | MXM(REG(XG), 0x00,    REG(XS)))

#define cvtqn_ld(XG, RM, DP)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C000699 | MXM(Tmm1,    Teax & (MOD(RM) == TPxx), TPxx))    \
        EMITW(0xF00007E3 | MXM(REG(XG), 0x00,    Tmm1))/* ^ == -1 if true */

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XG, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndqs_rr(W(XG), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvrqs_rr(XG, XS, mode)                                              \
        rnrqs_rr(W(XG), W(XS), mode)                                        \
        cvzqs_rr(W(XG), W(XG))

#endif /* RT_128 >= 2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P64_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

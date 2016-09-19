/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_128_H
#define RT_RTARCH_A64_128_H

#include "rtarch_a32_128.h"

#if defined (RT_SIMD_CODE)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64_128.h: Implementation of AArch64 fp64 NEON instructions.
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
/**********************************   MPE   ***********************************/
/******************************************************************************/

/**************************   packed generic (NEON)   *************************/

/* mov */

#define movqx_rr(RG, RM)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(RG), REG(RM), REG(RM)))

#define movqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

#define movqx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

/* and */

#define andqx_rr(RG, RM)                                                    \
        EMITW(0x4E201C00 | MXM(REG(RG), REG(RG), REG(RM)))

#define andqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E201C00 | MXM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annqx_rr(RG, RM)                                                    \
        EMITW(0x4E601C00 | MXM(REG(RG), REG(RM), REG(RG)))

#define annqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E601C00 | MXM(REG(RG), Tmm1,    REG(RG)))

/* orr */

#define orrqx_rr(RG, RM)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(RG), REG(RG), REG(RM)))

#define orrqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EA01C00 | MXM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorqx_rr(RG, RM)                                                    \
        EMITW(0x6E201C00 | MXM(REG(RG), REG(RG), REG(RM)))

#define xorqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E201C00 | MXM(REG(RG), REG(RG), Tmm1))

/* not */

#define notqx_rx(RM)                                                        \
        EMITW(0x6E205800 | MXM(REG(RM), REG(RM), 0x00))

/**************   packed double precision floating point (NEON)   *************/

/* neg */

#define negqs_rx(RM)                                                        \
        EMITW(0x6EE0F800 | MXM(REG(RM), REG(RM), 0x00))

/* add */

#define addqs_rr(RG, RM)                                                    \
        EMITW(0x4E60D400 | MXM(REG(RG), REG(RG), REG(RM)))

#define addqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60D400 | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subqs_rr(RG, RM)                                                    \
        EMITW(0x4EE0D400 | MXM(REG(RG), REG(RG), REG(RM)))

#define subqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE0D400 | MXM(REG(RG), REG(RG), Tmm1))

/* mul */

#define mulqs_rr(RG, RM)                                                    \
        EMITW(0x6E60DC00 | MXM(REG(RG), REG(RG), REG(RM)))

#define mulqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60DC00 | MXM(REG(RG), REG(RG), Tmm1))

/* div */

#define divqs_rr(RG, RM)                                                    \
        EMITW(0x6E60FC00 | MXM(REG(RG), REG(RG), REG(RM)))

#define divqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60FC00 | MXM(REG(RG), REG(RG), Tmm1))

/* sqr */

#define sqrqs_rr(RG, RM)                                                    \
        EMITW(0x6EE1F800 | MXM(REG(RG), REG(RM), 0x00))

#define sqrqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE1F800 | MXM(REG(RG), Tmm1,    0x00))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(RG, RM)                                                    \
        EMITW(0x4EE1D800 | MXM(REG(RG), REG(RM), 0x00))

#define rcsqs_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0x4E60FC00 | MXM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(RG), REG(RG), REG(RM)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(RG, RM)                                                    \
        EMITW(0x6EE1D800 | MXM(REG(RG), REG(RM), 0x00))

#define rssqs_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0x6E60DC00 | MXM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0x4EE0FC00 | MXM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(RG), REG(RG), REG(RM)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minqs_rr(RG, RM)                                                    \
        EMITW(0x4EE0F400 | MXM(REG(RG), REG(RG), REG(RM)))

#define minqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE0F400 | MXM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxqs_rr(RG, RM)                                                    \
        EMITW(0x4E60F400 | MXM(REG(RG), REG(RG), REG(RM)))

#define maxqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60F400 | MXM(REG(RG), REG(RG), Tmm1))

/* cmp */

#define ceqqs_rr(RG, RM)                                                    \
        EMITW(0x4E60E400 | MXM(REG(RG), REG(RG), REG(RM)))

#define ceqqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60E400 | MXM(REG(RG), REG(RG), Tmm1))

#define cneqs_rr(RG, RM)                                                    \
        EMITW(0x4E60E400 | MXM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x6E205800 | MXM(REG(RG), REG(RG), 0x00))

#define cneqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60E400 | MXM(REG(RG), REG(RG), Tmm1))                     \
        EMITW(0x6E205800 | MXM(REG(RG), REG(RG), 0x00))

#define cltqs_rr(RG, RM)                                                    \
        EMITW(0x6EE0E400 | MXM(REG(RG), REG(RM), REG(RG)))

#define cltqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE0E400 | MXM(REG(RG), Tmm1,    REG(RG)))

#define cleqs_rr(RG, RM)                                                    \
        EMITW(0x6E60E400 | MXM(REG(RG), REG(RM), REG(RG)))

#define cleqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60E400 | MXM(REG(RG), Tmm1,    REG(RG)))

#define cgtqs_rr(RG, RM)                                                    \
        EMITW(0x6EE0E400 | MXM(REG(RG), REG(RG), REG(RM)))

#define cgtqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE0E400 | MXM(REG(RG), REG(RG), Tmm1))

#define cgeqs_rr(RG, RM)                                                    \
        EMITW(0x6E60E400 | MXM(REG(RG), REG(RG), REG(RM)))

#define cgeqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60E400 | MXM(REG(RG), REG(RG), Tmm1))

/**************************   packed integer (NEON)   *************************/

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0x4EE19800 | MXM(REG(RG), REG(RM), 0x00))

#define rnzqs_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE19800 | MXM(REG(RG), Tmm1,    0x00))

#define cvzqs_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0x4EE1B800 | MXM(REG(RG), REG(RM), 0x00))

#define cvzqs_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE1B800 | MXM(REG(RG), Tmm1,    0x00))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(RG, RM)     /* round towards +inf */                       \
        EMITW(0x4EE18800 | MXM(REG(RG), REG(RM), 0x00))

#define rnpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE18800 | MXM(REG(RG), Tmm1,    0x00))

#define cvpqs_rr(RG, RM)     /* round towards +inf */                       \
        EMITW(0x4EE1A800 | MXM(REG(RG), REG(RM), 0x00))

#define cvpqs_ld(RG, RM, DP) /* round towards +inf */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE1A800 | MXM(REG(RG), Tmm1,    0x00))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(RG, RM)     /* round towards -inf */                       \
        EMITW(0x4E619800 | MXM(REG(RG), REG(RM), 0x00))

#define rnmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E619800 | MXM(REG(RG), Tmm1,    0x00))

#define cvmqs_rr(RG, RM)     /* round towards -inf */                       \
        EMITW(0x4E61B800 | MXM(REG(RG), REG(RM), 0x00))

#define cvmqs_ld(RG, RM, DP) /* round towards -inf */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E61B800 | MXM(REG(RG), Tmm1,    0x00))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(RG, RM)     /* round towards near */                       \
        EMITW(0x4E618800 | MXM(REG(RG), REG(RM), 0x00))

#define rnnqs_ld(RG, RM, DP) /* round towards near */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E618800 | MXM(REG(RG), Tmm1,    0x00))

#define cvnqs_rr(RG, RM)     /* round towards near */                       \
        EMITW(0x4E61A800 | MXM(REG(RG), REG(RM), 0x00))

#define cvnqs_ld(RG, RM, DP) /* round towards near */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E61A800 | MXM(REG(RG), Tmm1,    0x00))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(RG, RM)     /* round towards near */                       \
        cvtqn_rr(W(RG), W(RM))

#define cvnqn_ld(RG, RM, DP) /* round towards near */                       \
        cvtqn_ld(W(RG), W(RM), W(DP))

/* add */

#define addqx_rr(RG, RM)                                                    \
        EMITW(0x4EE08400 | MXM(REG(RG), REG(RG), REG(RM)))

#define addqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE08400 | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subqx_rr(RG, RM)                                                    \
        EMITW(0x6EE08400 | MXM(REG(RG), REG(RG), REG(RM)))

#define subqx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE08400 | MXM(REG(RG), REG(RG), Tmm1))

/* shl */

#define shlqx_ri(RM, IM)                                                    \
        EMITW(0x4F405400 | MXM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x3F & VAL(IM)) << 16)

#define shlqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E080400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(RG), REG(RG), Tmm1))

/* shr */

#define shrqx_ri(RM, IM) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(RM), REG(RM), 0x00) |                    \
        (+(VAL(IM) == 0) & 0x00005000) | (+(VAL(IM) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IM)) << 16)

#define shrqx_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E080400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(RG), REG(RG), Tmm1))

#define shrqn_ri(RM, IM) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(RM), REG(RM), 0x00) |                    \
        (+(VAL(IM) == 0) & 0x00005000) | (+(VAL(IM) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IM)) << 16)

#define shrqn_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E080400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(RG), REG(RG), Tmm1))

/**************************   helper macros (NEON)   **************************/

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(RG, RM)                                                    \
        EMITW(0x6EE19800 | MXM(REG(RG), REG(RM), 0x00))

#define rndqs_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE19800 | MXM(REG(RG), Tmm1,    0x00))

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
        EMITW(0x4E61D800 | MXM(REG(RG), REG(RM), 0x00))

#define cvtqn_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E61D800 | MXM(REG(RG), Tmm1,    0x00))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(RG, RM, mode)                                              \
        EMITW(0x4E618800 | MXM(REG(RG), REG(RM), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvrqs_rr(RG, RM, mode)                                              \
        EMITW(0x4E61A800 | MXM(REG(RG), REG(RM), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A64_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

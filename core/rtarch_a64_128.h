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
 * DP - displacement value (of given size DP, DF, DG, DH, DV)
 * IS - immediate value (is used as a second or first source)
 * IT - immediate value (is used as a third or second source)
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

#define movqx_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))

#define movqx_ld(XD, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DP), B2(DP), P2(DP)))

#define movqx_st(XS, MD, DP)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DP), B2(DP), P2(DP)))

/* and */

#define andqx_rr(XG, XS)                                                    \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), REG(XS)))

#define andqx_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), Tmm1))

/* ann (G = ~G & S) */

#define annqx_rr(XG, XS)                                                    \
        EMITW(0x4E601C00 | MXM(REG(XG), REG(XS), REG(XG)))

#define annqx_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E601C00 | MXM(REG(XG), Tmm1,    REG(XG)))

/* orr */

#define orrqx_rr(XG, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrqx_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), Tmm1))

/* orn (G = ~G | S) */

#define ornqx_rr(XG, XS)                                                    \
        EMITW(0x4EE01C00 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornqx_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE01C00 | MXM(REG(XG), Tmm1,    REG(XG)))

/* xor */

#define xorqx_rr(XG, XS)                                                    \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorqx_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), Tmm1))

/* not */

#define notqx_rx(XG)                                                        \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

/**************   packed double precision floating point (NEON)   *************/

/* neg */

#define negqs_rx(XG)                                                        \
        EMITW(0x6EE0F800 | MXM(REG(XG), REG(XG), 0x00))

/* add */

#define addqs_rr(XG, XS)                                                    \
        EMITW(0x4E60D400 | MXM(REG(XG), REG(XG), REG(XS)))

#define addqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60D400 | MXM(REG(XG), REG(XG), Tmm1))

/* sub */

#define subqs_rr(XG, XS)                                                    \
        EMITW(0x4EE0D400 | MXM(REG(XG), REG(XG), REG(XS)))

#define subqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE0D400 | MXM(REG(XG), REG(XG), Tmm1))

/* mul */

#define mulqs_rr(XG, XS)                                                    \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#define mulqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), Tmm1))

/* div */

#define divqs_rr(XG, XS)                                                    \
        EMITW(0x6E60FC00 | MXM(REG(XG), REG(XG), REG(XS)))

#define divqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60FC00 | MXM(REG(XG), REG(XG), Tmm1))

/* sqr */

#define sqrqs_rr(XD, XS)                                                    \
        EMITW(0x6EE1F800 | MXM(REG(XD), REG(XS), 0x00))

#define sqrqs_ld(XD, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE1F800 | MXM(REG(XD), Tmm1,    0x00))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(XD, XS)                                                    \
        EMITW(0x4EE1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsqs_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0x4E60FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(XD, XS)                                                    \
        EMITW(0x6EE1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssqs_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0x6E60DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EE0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minqs_rr(XG, XS)                                                    \
        EMITW(0x4EE0F400 | MXM(REG(XG), REG(XG), REG(XS)))

#define minqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE0F400 | MXM(REG(XG), REG(XG), Tmm1))

/* max */

#define maxqs_rr(XG, XS)                                                    \
        EMITW(0x4E60F400 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60F400 | MXM(REG(XG), REG(XG), Tmm1))

/* cmp */

#define ceqqs_rr(XG, XS)                                                    \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), Tmm1))

#define cneqs_rr(XG, XS)                                                    \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

#define cneqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), Tmm1))                     \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

#define cltqs_rr(XG, XS)                                                    \
        EMITW(0x6EE0E400 | MXM(REG(XG), REG(XS), REG(XG)))

#define cltqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE0E400 | MXM(REG(XG), Tmm1,    REG(XG)))

#define cleqs_rr(XG, XS)                                                    \
        EMITW(0x6E60E400 | MXM(REG(XG), REG(XS), REG(XG)))

#define cleqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60E400 | MXM(REG(XG), Tmm1,    REG(XG)))

#define cgtqs_rr(XG, XS)                                                    \
        EMITW(0x6EE0E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE0E400 | MXM(REG(XG), REG(XG), Tmm1))

#define cgeqs_rr(XG, XS)                                                    \
        EMITW(0x6E60E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgeqs_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6E60E400 | MXM(REG(XG), REG(XG), Tmm1))

/**************************   packed integer (NEON)   *************************/

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE19800 | MXM(REG(XD), REG(XS), 0x00))

#define rnzqs_ld(XD, MS, DP) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE19800 | MXM(REG(XD), Tmm1,    0x00))

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE1B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvzqs_ld(XD, MS, DP) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE1B800 | MXM(REG(XD), Tmm1,    0x00))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE18800 | MXM(REG(XD), REG(XS), 0x00))

#define rnpqs_ld(XD, MS, DP) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE18800 | MXM(REG(XD), Tmm1,    0x00))

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE1A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvpqs_ld(XD, MS, DP) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE1A800 | MXM(REG(XD), Tmm1,    0x00))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E619800 | MXM(REG(XD), REG(XS), 0x00))

#define rnmqs_ld(XD, MS, DP) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E619800 | MXM(REG(XD), Tmm1,    0x00))

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E61B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvmqs_ld(XD, MS, DP) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E61B800 | MXM(REG(XD), Tmm1,    0x00))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00))

#define rnnqs_ld(XD, MS, DP) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E618800 | MXM(REG(XD), Tmm1,    0x00))

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvnqs_ld(XD, MS, DP) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E61A800 | MXM(REG(XD), Tmm1,    0x00))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        cvtqn_rr(W(XD), W(XS))

#define cvnqn_ld(XD, MS, DP) /* round towards near */                       \
        cvtqn_ld(W(XD), W(MS), W(DP))

/* add */

#define addqx_rr(XG, XS)                                                    \
        EMITW(0x4EE08400 | MXM(REG(XG), REG(XG), REG(XS)))

#define addqx_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4EE08400 | MXM(REG(XG), REG(XG), Tmm1))

/* sub */

#define subqx_rr(XG, XS)                                                    \
        EMITW(0x6EE08400 | MXM(REG(XG), REG(XG), REG(XS)))

#define subqx_ld(XG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE08400 | MXM(REG(XG), REG(XG), Tmm1))

/* shl */

#define shlqx_ri(XG, IS)                                                    \
        EMITW(0x4F405400 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)

#define shlqx_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E080400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), Tmm1))

/* shr */

#define shrqx_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)

#define shrqx_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E080400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), Tmm1))

#define shrqn_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)

#define shrqn_ld(XG, MS, DP) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E080400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XG), REG(XG), Tmm1))

/**************************   helper macros (NEON)   **************************/

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        EMITW(0x6EE19800 | MXM(REG(XD), REG(XS), 0x00))

#define rndqs_ld(XD, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x6EE19800 | MXM(REG(XD), Tmm1,    0x00))

#define cvtqs_rr(XD, XS)                                                    \
        rndqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvtqs_ld(XD, MS, DP)                                                \
        rndqs_ld(W(XD), W(MS), W(DP))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtqn_rr(XD, XS)                                                    \
        EMITW(0x4E61D800 | MXM(REG(XD), REG(XS), 0x00))

#define cvtqn_ld(XD, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E61D800 | MXM(REG(XD), Tmm1,    0x00))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvrqs_rr(XD, XS, mode)                                              \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A64_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

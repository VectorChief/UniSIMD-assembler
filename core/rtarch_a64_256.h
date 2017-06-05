/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_256_H
#define RT_RTARCH_A64_256_H

#include "rtarch_a32_256.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64_256.h: Implementation of AArch64 fp64 NEON instructions (pairs).
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

#if defined (RT_256) && (RT_256 != 0) && (RT_SIMD_COMPAT_XMM > 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/**************************   packed generic (NEON)   *************************/

/* mov (D = S) */

#define movqx_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movqx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(RYG(XD), MOD(MS), VYL(DS), B2(DS), P2(DS)))

#define movqx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VYL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(RYG(XS), MOD(MD), VYL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvqx_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x6EA01C00 | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvqx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VYL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VYL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VYL(DG), B2(DG), P2(DG)))

/* and (G = G & S) */

#define andqx_rr(XG, XS)                                                    \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E201C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define andqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E201C00 | MXM(RYG(XG), RYG(XG), TmmM))

/* ann (G = ~G & S) */

#define annqx_rr(XG, XS)                                                    \
        EMITW(0x4E601C00 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x4E601C00 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define annqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E601C00 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E601C00 | MXM(RYG(XG), TmmM,    RYG(XG)))

/* orr (G = G | S) */

#define orrqx_rr(XG, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define orrqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA01C00 | MXM(RYG(XG), RYG(XG), TmmM))

/* orn (G = ~G | S) */

#define ornqx_rr(XG, XS)                                                    \
        EMITW(0x4EE01C00 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x4EE01C00 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define ornqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE01C00 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE01C00 | MXM(RYG(XG), TmmM,    RYG(XG)))

/* xor (G = G ^ S) */

#define xorqx_rr(XG, XS)                                                    \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E201C00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define xorqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E201C00 | MXM(RYG(XG), RYG(XG), TmmM))

/* not (G = ~G) */

#define notqx_rx(XG)                                                        \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))                     \
        EMITW(0x6E205800 | MXM(RYG(XG), RYG(XG), 0x00))

/**************   packed double precision floating point (NEON)   *************/

/* neg (G = -G) */

#define negqs_rx(XG)                                                        \
        EMITW(0x6EE0F800 | MXM(REG(XG), REG(XG), 0x00))                     \
        EMITW(0x6EE0F800 | MXM(RYG(XG), RYG(XG), 0x00))

/* add (G = G + S) */

#define addqs_rr(XG, XS)                                                    \
        EMITW(0x4E60D400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E60D400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60D400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60D400 | MXM(RYG(XG), RYG(XG), TmmM))

/* sub (G = G - S) */

#define subqs_rr(XG, XS)                                                    \
        EMITW(0x4EE0D400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EE0D400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE0D400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE0D400 | MXM(RYG(XG), RYG(XG), TmmM))

/* mul (G = G * S) */

#define mulqs_rr(XG, XS)                                                    \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mulqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60DC00 | MXM(RYG(XG), RYG(XG), TmmM))

/* div (G = G / S) */

#define divqs_rr(XG, XS)                                                    \
        EMITW(0x6E60FC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E60FC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define divqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60FC00 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60FC00 | MXM(RYG(XG), RYG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqrqs_rr(XD, XS)                                                    \
        EMITW(0x6EE1F800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE1F800 | MXM(RYG(XD), RYG(XS), 0x00))

#define sqrqs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE1F800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE1F800 | MXM(RYG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceqs_rr(XD, XS)                                                    \
        EMITW(0x4EE1D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rcsqs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E60FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E60FC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseqs_rr(XD, XS)                                                    \
        EMITW(0x6EE1D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE1D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rssqs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E60DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EE0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x4EE0FC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaqs_rr(XG, XS, XT)                                                \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4E60CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsqs_rr(XG, XS, XT)                                                \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4EE0CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minqs_rr(XG, XS)                                                    \
        EMITW(0x4EE0F400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EE0F400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define minqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE0F400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE0F400 | MXM(RYG(XG), RYG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxqs_rr(XG, XS)                                                    \
        EMITW(0x4E60F400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E60F400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define maxqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60F400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60F400 | MXM(RYG(XG), RYG(XG), TmmM))

/* cmp (G = G ? S) */

#define ceqqs_rr(XG, XS)                                                    \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E60E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define ceqqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60E400 | MXM(RYG(XG), RYG(XG), TmmM))

#define cneqs_rr(XG, XS)                                                    \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))                     \
        EMITW(0x4E60E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0x6E205800 | MXM(RYG(XG), RYG(XG), 0x00))

#define cneqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E60E400 | MXM(RYG(XG), RYG(XG), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XG), RYG(XG), 0x00))

#define cltqs_rr(XG, XS)                                                    \
        EMITW(0x6EE0E400 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x6EE0E400 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define cltqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0E400 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0E400 | MXM(RYG(XG), TmmM,    RYG(XG)))

#define cleqs_rr(XG, XS)                                                    \
        EMITW(0x6E60E400 | MXM(REG(XG), REG(XS), REG(XG)))                  \
        EMITW(0x6E60E400 | MXM(RYG(XG), RYG(XS), RYG(XG)))

#define cleqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60E400 | MXM(REG(XG), TmmM,    REG(XG)))                  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60E400 | MXM(RYG(XG), TmmM,    RYG(XG)))

#define cgtqs_rr(XG, XS)                                                    \
        EMITW(0x6EE0E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6EE0E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgtqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0E400 | MXM(RYG(XG), RYG(XG), TmmM))

#define cgeqs_rr(XG, XS)                                                    \
        EMITW(0x6E60E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E60E400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define cgeqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E60E400 | MXM(RYG(XG), RYG(XG), TmmM))

/**************************   packed integer (NEON)   *************************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnzqs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE19800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE19800 | MXM(RYG(XD), TmmM,    0x00))

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE1B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1B800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE18800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE18800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE18800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE18800 | MXM(RYG(XD), TmmM,    0x00))

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE1A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1A800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E619800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E619800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E619800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E619800 | MXM(RYG(XD), TmmM,    0x00))

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E61B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61B800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E618800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnnqs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E618800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E618800 | MXM(RYG(XD), TmmM,    0x00))

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvnqs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61A800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        cvtqn_rr(W(XD), W(XS))

#define cvnqn_ld(XD, MS, DS) /* round towards near */                       \
        cvtqn_ld(W(XD), W(MS), W(DS))

/* add (G = G + S) */

#define addqx_rr(XG, XS)                                                    \
        EMITW(0x4EE08400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4EE08400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define addqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE08400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE08400 | MXM(RYG(XG), RYG(XG), TmmM))

/* sub (G = G - S) */

#define subqx_rr(XG, XS)                                                    \
        EMITW(0x6EE08400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6EE08400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define subqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE08400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE08400 | MXM(RYG(XG), RYG(XG), TmmM))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlqx_ri(XG, IS)                                                    \
        EMITW(0x4F405400 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)    \
        EMITW(0x4F405400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrqx_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (M(VAL(IS) == 0) & 0x00005000) | (M(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)    \
        EMITW(0x4F400400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
        (M(VAL(IS) == 0) & 0x00005000) | (M(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    RYG(XS), 0x00))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))


#define shrqn_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (M(VAL(IS) == 0) & 0x00005000) | (M(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)    \
        EMITW(0x4F400400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
        (M(VAL(IS) == 0) & 0x00005000) | (M(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x4EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    RYG(XS), 0x00))                     \
        EMITW(0x4EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

/**************************   helper macros (NEON)   **************************/

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        EMITW(0x6EE19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rndqs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE19800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE19800 | MXM(RYG(XD), TmmM,    0x00))

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
        EMITW(0x4E61D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtqn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61D800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61D800 | MXM(RYG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E618800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvrqs_rr(XD, XS, mode)                                              \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E61A800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A64_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

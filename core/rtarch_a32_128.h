/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_128_H
#define RT_RTARCH_A32_128_H

#include "rtarch_a64.h"

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       16
#define RT_SIMD_WIDTH32     4
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH64     2
#define RT_SIMD_SET64(s, v) s[0]=s[1]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a32_128.h: Implementation of AArch64 fp32 NEON instructions.
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

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 5 | (reg))

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 5 | (reg))

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0xFFF0) << 6)
#define C20(br, dp) EMPTY

#define B22(br) TPxx
#define P22(dp) (0x00000000)
#define C22(br, dp) EMITW(0x52800000 | MXM(TDxx,    0x00,    0x00) |        \
                             (0xFFF0 & (dp)) << 5)                          \
                    EMITW(0x72A00000 | MXM(TDxx,    0x00,    0x00) |        \
                             (0x7FFF & (dp) >> 16) << 5)                    \
                    EMITW(0x0B000000 | MXM(TPxx,    (br),    TDxx) | ADR)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tmm1    0x1F  /* v31 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* v0 */
#define Xmm1    0x01, 0x00, EMPTY       /* v1 */
#define Xmm2    0x02, 0x00, EMPTY       /* v2 */
#define Xmm3    0x03, 0x00, EMPTY       /* v3 */
#define Xmm4    0x04, 0x00, EMPTY       /* v4 */
#define Xmm5    0x05, 0x00, EMPTY       /* v5 */
#define Xmm6    0x06, 0x00, EMPTY       /* v6 */
#define Xmm7    0x07, 0x00, EMPTY       /* v7 */
#define Xmm8    0x08, 0x00, EMPTY       /* v8 */
#define Xmm9    0x09, 0x00, EMPTY       /* v9 */
#define XmmA    0x0A, 0x00, EMPTY       /* v10 */
#define XmmB    0x0B, 0x00, EMPTY       /* v11 */
#define XmmC    0x0C, 0x00, EMPTY       /* v12 */
#define XmmD    0x0D, 0x00, EMPTY       /* v13 */
#define XmmE    0x0E, 0x00, EMPTY       /* v14 */
#define XmmF    0x0F, 0x00, EMPTY       /* v15 */

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/**************************   packed generic (NEON)   *************************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))

#define movox_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movox_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))


#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, DS is SIMD-aligned */     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x8B000000 | MRM(REG(RD), MOD(MS), TDxx))

/* and (G = G & S) */

#define andox_rr(XG, XS)                                                    \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), REG(XS)))

#define andox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E201C00 | MXM(REG(XG), REG(XG), Tmm1))

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        EMITW(0x4E601C00 | MXM(REG(XG), REG(XS), REG(XG)))

#define annox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E601C00 | MXM(REG(XG), Tmm1,    REG(XG)))

/* orr (G = G | S) */

#define orrox_rr(XG, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA01C00 | MXM(REG(XG), REG(XG), Tmm1))

/* orn (G = ~G | S) */

#define ornox_rr(XG, XS)                                                    \
        EMITW(0x4EE01C00 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE01C00 | MXM(REG(XG), Tmm1,    REG(XG)))

/* xor (G = G ^ S) */

#define xorox_rr(XG, XS)                                                    \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E201C00 | MXM(REG(XG), REG(XG), Tmm1))

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

/**************   packed single precision floating point (NEON)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        EMITW(0x6EA0F800 | MXM(REG(XG), REG(XG), 0x00))

/* add (G = G + S) */

#define addos_rr(XG, XS)                                                    \
        EMITW(0x4E20D400 | MXM(REG(XG), REG(XG), REG(XS)))

#define addos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20D400 | MXM(REG(XG), REG(XG), Tmm1))

/* sub (G = G - S) */

#define subos_rr(XG, XS)                                                    \
        EMITW(0x4EA0D400 | MXM(REG(XG), REG(XG), REG(XS)))

#define subos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA0D400 | MXM(REG(XG), REG(XG), Tmm1))

/* mul (G = G * S) */

#define mulos_rr(XG, XS)                                                    \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#define mulos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), Tmm1))

/* div (G = G / S) */

#define divos_rr(XG, XS)                                                    \
        EMITW(0x6E20FC00 | MXM(REG(XG), REG(XG), REG(XS)))

#define divos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20FC00 | MXM(REG(XG), REG(XG), Tmm1))

/* sqr */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0x6EA1F800 | MXM(REG(XD), REG(XS), 0x00))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA1F800 | MXM(REG(XD), Tmm1,    0x00))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        EMITW(0x4EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0x4E20FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        EMITW(0x6EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0x6E20DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EA0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), Tmm1))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), Tmm1))

#endif /* RT_SIMD_COMPAT_FMS */

/* min */

#define minos_rr(XG, XS)                                                    \
        EMITW(0x4EA0F400 | MXM(REG(XG), REG(XG), REG(XS)))

#define minos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA0F400 | MXM(REG(XG), REG(XG), Tmm1))

/* max */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0x4E20F400 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20F400 | MXM(REG(XG), REG(XG), Tmm1))

/* cmp */

#define ceqos_rr(XG, XS)                                                    \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), Tmm1))

#define cneos_rr(XG, XS)                                                    \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

#define cneos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E20E400 | MXM(REG(XG), REG(XG), Tmm1))                     \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

#define cltos_rr(XG, XS)                                                    \
        EMITW(0x6EA0E400 | MXM(REG(XG), REG(XS), REG(XG)))

#define cltos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0E400 | MXM(REG(XG), Tmm1,    REG(XG)))

#define cleos_rr(XG, XS)                                                    \
        EMITW(0x6E20E400 | MXM(REG(XG), REG(XS), REG(XG)))

#define cleos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20E400 | MXM(REG(XG), Tmm1,    REG(XG)))

#define cgtos_rr(XG, XS)                                                    \
        EMITW(0x6EA0E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0E400 | MXM(REG(XG), REG(XG), Tmm1))

#define cgeos_rr(XG, XS)                                                    \
        EMITW(0x6E20E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgeos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6E20E400 | MXM(REG(XG), REG(XG), Tmm1))

/**************************   packed integer (NEON)   *************************/

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA19800 | MXM(REG(XD), REG(XS), 0x00))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA19800 | MXM(REG(XD), Tmm1,    0x00))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA1B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1B800 | MXM(REG(XD), Tmm1,    0x00))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA18800 | MXM(REG(XD), REG(XS), 0x00))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA18800 | MXM(REG(XD), Tmm1,    0x00))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA1A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1A800 | MXM(REG(XD), Tmm1,    0x00))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E219800 | MXM(REG(XD), REG(XS), 0x00))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E219800 | MXM(REG(XD), Tmm1,    0x00))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E21B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21B800 | MXM(REG(XD), Tmm1,    0x00))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E218800 | MXM(REG(XD), Tmm1,    0x00))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21A800 | MXM(REG(XD), Tmm1,    0x00))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        EMITW(0x4EA08400 | MXM(REG(XG), REG(XG), REG(XS)))

#define addox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA08400 | MXM(REG(XG), REG(XG), Tmm1))

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        EMITW(0x6EA08400 | MXM(REG(XG), REG(XG), REG(XS)))

#define subox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA08400 | MXM(REG(XG), REG(XG), Tmm1))

/* shl (G = G << S) */

#define shlox_ri(XG, IS)                                                    \
        EMITW(0x4F205400 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), Tmm1))

/* shr (G = G >> S) */

#define shrox_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F200400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), Tmm1))

#define shron_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F200400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XG), REG(XG), Tmm1))

/**************************   helper macros (NEON)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x04    /*  all satisfy the condition */

#define movms_rr(XD, XS) /* not portable, do not use outside */             \
        EMITW(0x4EB1B800 | MXM(Tmm1,    REG(XS), 0x00))                     \
        EMITW(0x0E043C00 | MXM(REG(XD), Tmm1,    0x00))

#define CHECK_MASK(lb, mask, XS) /* destroys Reax */                        \
        movms_rr(Reax, W(XS))                                               \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        jezxx_lb(lb)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x02    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x01    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x06    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x05    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x07    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x06    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x05    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x07    /* round towards zero */

#define fpscr_ld(RS) /* not portable, do not use outside */                 \
        EMITW(0xD51B4400 | MRM(REG(RS), 0x00,    0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xD53B4400 | MRM(REG(RD), 0x00,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0x52A00000 | MRM(TIxx,    0x00,    0x00) |                    \
                           RT_SIMD_MODE_##mode << 11)                       \
        EMITW(0xD51B4400 | MRM(TIxx,    0x00,    0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xD51B4400 | MRM(TNxx,    0x00,    0x00))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xD51B4400 | MRM(TNxx+(RT_SIMD_MODE_##mode&3), 0x00, 0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xD51B4400 | MRM(TNxx,    0x00,    0x00))

#endif /* RT_SIMD_FAST_FCTRL */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0x6EA19800 | MXM(REG(XD), REG(XS), 0x00))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA19800 | MXM(REG(XD), Tmm1,    0x00))

#define cvtos_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvtos_ld(XD, MS, DS)                                                \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        EMITW(0x4E21D800 | MXM(REG(XD), REG(XS), 0x00))

#define cvton_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21D800 | MXM(REG(XD), Tmm1,    0x00))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvros_rr(XD, XS, mode)                                              \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3D800000 | MXM(Tmm1,    Teax,    0x00))

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x3DC00000 | MXM(Tmm1,    Teax,    0x00))

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_128_H
#define RT_RTARCH_P32_128_H

#include "rtarch_p32.h"

#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_128.h: Implementation of PowerPC 32-bit SIMD-128 instructions.
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

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 11 | (ren) << 16 | (reg) << 21)

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 16 | (reg) << 21)

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0x7FF0))
#define C20(br, dp) EMPTY

#define B21(br) (br)
#define P21(dp) (0x44000214 | TDxx << 11)
#define C21(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFF0 & (dp)))

#define B22(br) (br)
#define P22(dp) (0x44000214 | TDxx << 11)
#define C22(br, dp) EMITW(0x64000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x60000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFF0 & (dp)))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TmmS    0x18  /* vr24, SIGN */
#define TmmQ    0x19  /* vr25, QNAN */
#define TmmA    0x1A  /* vr26, +1.0 */
#define TmmB    0x1B  /* vr27, -0.5 */
#define TmmC    0x1C  /* vr28 */
#define TmmD    0x1D  /* vr29 */
#define Tmm1    0x1F  /* vr31 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* vr0 */
#define Xmm1    0x01, 0x00, EMPTY       /* vr1 */
#define Xmm2    0x02, 0x00, EMPTY       /* vr2 */
#define Xmm3    0x03, 0x00, EMPTY       /* vr3 */
#define Xmm4    0x04, 0x00, EMPTY       /* vr4 */
#define Xmm5    0x05, 0x00, EMPTY       /* vr5 */
#define Xmm6    0x06, 0x00, EMPTY       /* vr6 */
#define Xmm7    0x07, 0x00, EMPTY       /* vr7 */
#define Xmm8    0x08, 0x00, EMPTY       /* vr8 */
#define Xmm9    0x09, 0x00, EMPTY       /* vr9 */
#define XmmA    0x0A, 0x00, EMPTY       /* vr10 */
#define XmmB    0x0B, 0x00, EMPTY       /* vr11 */
#define XmmC    0x0C, 0x00, EMPTY       /* vr12 */
#define XmmD    0x0D, 0x00, EMPTY       /* vr13 */
#define XmmE    0x0E, 0x00, EMPTY       /* vr14 */
#define XmmF    0x0F, 0x00, EMPTY       /* vr15 */

/******************************************************************************/
/**********************************   VMX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        EMITW(0x10000484 | MXM(REG(RG), REG(RM), REG(RM)))

#define movpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(REG(RG), TPxx,    0x00))

#define movpx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0001CE | MXM(REG(RG), TPxx,    0x00))

#define adrpx_ld(RG, RM, DP) /* RG is a core reg, DP is SIMD-aligned */     \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITW(0x10000404 | MXM(REG(RG), REG(RG), REG(RM)))

#define andpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x10000404 | MXM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITW(0x10000444 | MXM(REG(RG), REG(RM), REG(RG)))

#define annpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x10000444 | MXM(REG(RG), Tmm1,    REG(RG)))

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITW(0x10000484 | MXM(REG(RG), REG(RG), REG(RM)))

#define orrpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x10000484 | MXM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITW(0x100004C4 | MXM(REG(RG), REG(RG), REG(RM)))

#define xorpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100004C4 | MXM(REG(RG), REG(RG), Tmm1))

/**************   packed single precision floating point (SIMD)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITW(0x1000000A | MXM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000000A | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITW(0x1000004A | MXM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000004A | MXM(REG(RG), REG(RG), Tmm1))

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITW(0x1000002E | MXM(REG(RG), REG(RG), TmmS) | REG(RM) << 6)

#define mulps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000002E | MXM(REG(RG), REG(RG), TmmS) | Tmm1 << 6)

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITW(0x1000010A | MXM(TmmC,    0x00,    REG(RM)))                  \
        EMITW(0x1000002F | MXM(TmmD,    TmmC,    TmmA) | REG(RM) << 6)      \
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmC) | TmmD << 6)         \
        EMITW(0x1000002E | MXM(REG(RG), REG(RG), TmmS) | TmmD << 6)

#define divps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000010A | MXM(TmmC,    0x00,    Tmm1))                     \
        EMITW(0x1000002F | MXM(TmmD,    TmmC,    TmmA) | Tmm1 << 6)         \
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmC) | TmmD << 6)         \
        EMITW(0x1000002E | MXM(REG(RG), REG(RG), TmmS) | TmmD << 6)

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITW(0x1000014A | MXM(TmmC,    0x00,    REG(RM)))                  \
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmS) | TmmC << 6)         \
        EMITW(0x1000002E | MXM(REG(RG), TmmC,    TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | REG(RM) << 6)      \
        EMITW(0x1000002F | MXM(REG(RG), TmmD,    TmmC) | REG(RG) << 6)      \
        EMITW(0x1000002E | MXM(REG(RG), REG(RG), TmmS) | REG(RM) << 6)

#define sqrps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000014A | MXM(TmmC,    0x00,    Tmm1))                     \
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmS) | TmmC << 6)         \
        EMITW(0x1000002E | MXM(REG(RG), TmmC,    TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | Tmm1 << 6)         \
        EMITW(0x1000002F | MXM(REG(RG), TmmD,    TmmC) | REG(RG) << 6)      \
        EMITW(0x1000002E | MXM(REG(RG), REG(RG), TmmS) | REG(RM) << 6)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        EMITW(0x1000010A | MXM(REG(RG), 0x00,    REG(RM)))

#define rcsps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0x1000002F | MXM(REG(RM), REG(RG), TmmA) | REG(RM) << 6)      \
        EMITW(0x1000002E | MXM(REG(RG), REG(RG), REG(RG)) | REG(RM) << 6)

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq */

#define rseps_rr(RG, RM)                                                    \
        EMITW(0x1000014A | MXM(REG(RG), 0x00,    REG(RM)))

#define rssps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0x1000002E | MXM(TmmD,    REG(RG), TmmS) | REG(RG) << 6)      \
        EMITW(0x1000002E | MXM(TmmC,    REG(RG), TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | REG(RM) << 6)      \
        EMITW(0x1000002F | MXM(REG(RG), TmmD,    REG(RG)) | TmmC << 6)

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITW(0x1000044A | MXM(REG(RG), REG(RG), REG(RM)))

#define minps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000044A | MXM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITW(0x1000040A | MXM(REG(RG), REG(RG), REG(RM)))

#define maxps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000040A | MXM(REG(RG), REG(RG), Tmm1))

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITW(0x100000C6 | MXM(REG(RG), REG(RG), REG(RM)))

#define ceqps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100000C6 | MXM(REG(RG), REG(RG), Tmm1))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0x100000C6 | MXM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x10000504 | MXM(REG(RG), REG(RG), REG(RG)))

#define cneps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100000C6 | MXM(REG(RG), REG(RG), Tmm1))                     \
        EMITW(0x10000504 | MXM(REG(RG), REG(RG), REG(RG)))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0x100002C6 | MXM(REG(RG), REG(RM), REG(RG)))

#define cltps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100002C6 | MXM(REG(RG), Tmm1,    REG(RG)))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0x100001C6 | MXM(REG(RG), REG(RM), REG(RG)))

#define cleps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100001C6 | MXM(REG(RG), Tmm1,    REG(RG)))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0x100002C6 | MXM(REG(RG), REG(RG), REG(RM)))

#define cgtps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100002C6 | MXM(REG(RG), REG(RG), Tmm1))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0x100001C6 | MXM(REG(RG), REG(RG), REG(RM)))

#define cgeps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100001C6 | MXM(REG(RG), REG(RG), Tmm1))

/**************************   packed integer (SIMD)   *************************/

/* cvz
 * rounding mode is encoded directly */

#define rnzps_rr(RG, RM)     /* round towards zero  (fp-to-signed-int) */   \
        EMITW(0x1000024A | MXM(REG(RG), 0x00,    REG(RM)))

#define rnzps_ld(RG, RM, DP) /* round towards zero  (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000024A | MXM(REG(RG), 0x00,    Tmm1))

#define cvzps_rr(RG, RM)     /* round towards zero  (fp-to-signed-int) */   \
        EMITW(0x100003CA | MXM(REG(RG), 0x00,    REG(RM)))

#define cvzps_ld(RG, RM, DP) /* round towards zero  (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100003CA | MXM(REG(RG), 0x00,    Tmm1))

/* cvp
 * rounding mode is encoded directly */

#define rnpps_rr(RG, RM)     /* round towards +inf  (fp-to-signed-int) */   \
        EMITW(0x1000028A | MXM(REG(RG), 0x00,    REG(RM)))

#define rnpps_ld(RG, RM, DP) /* round towards +inf  (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000028A | MXM(REG(RG), 0x00,    Tmm1))

#define cvpps_rr(RG, RM)     /* round towards +inf  (fp-to-signed-int) */   \
        rnpps_rr(W(RG), W(RM))                                              \
        cvzps_rr(W(RG), W(RG))

#define cvpps_ld(RG, RM, DP) /* round towards +inf  (fp-to-signed-int) */   \
        rnpps_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

/* cvm
 * rounding mode is encoded directly */

#define rnmps_rr(RG, RM)     /* round towards -inf  (fp-to-signed-int) */   \
        EMITW(0x100002CA | MXM(REG(RG), 0x00,    REG(RM)))

#define rnmps_ld(RG, RM, DP) /* round towards -inf  (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x100002CA | MXM(REG(RG), 0x00,    Tmm1))

#define cvmps_rr(RG, RM)     /* round towards -inf  (fp-to-signed-int) */   \
        rnmps_rr(W(RG), W(RM))                                              \
        cvzps_rr(W(RG), W(RG))

#define cvmps_ld(RG, RM, DP) /* round towards -inf  (fp-to-signed-int) */   \
        rnmps_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

/* cvn
 * rounding mode is encoded directly */

#define rnnps_rr(RG, RM)     /* round to nearest    (fp-to-signed-int) */   \
        EMITW(0x1000020A | MXM(REG(RG), 0x00,    REG(RM)))

#define rnnps_ld(RG, RM, DP) /* round to nearest    (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000020A | MXM(REG(RG), 0x00,    Tmm1))

#define cvnps_rr(RG, RM)     /* round to nearest    (fp-to-signed-int) */   \
        rnnps_rr(W(RG), W(RM))                                              \
        cvzps_rr(W(RG), W(RG))

#define cvnps_ld(RG, RM, DP) /* round to nearest    (fp-to-signed-int) */   \
        rnnps_ld(W(RG), W(RM), W(DP))                                       \
        cvzps_rr(W(RG), W(RG))

#define cvnpn_rr(RG, RM)     /* round to nearest    (signed-int-to-fp) */   \
        EMITW(0x1000034A | MXM(REG(RG), 0x00,    REG(RM)))

#define cvnpn_ld(RG, RM, DP) /* round to nearest    (signed-int-to-fp) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000034A | MXM(REG(RG), 0x00,    Tmm1))

/* add */

#define addpx_rr(RG, RM)                                                    \
        EMITW(0x10000080 | MXM(REG(RG), REG(RG), REG(RM)))

#define addpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x10000080 | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subpx_rr(RG, RM)                                                    \
        EMITW(0x10000480 | MXM(REG(RG), REG(RG), REG(RM)))

#define subpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x10000480 | MXM(REG(RG), REG(RG), Tmm1))

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IM)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(RM), REG(RM), Tmm1))

#define shlpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000028C | MXM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0x10000184 | MXM(REG(RG), REG(RG), Tmm1))

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IM)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(RM), REG(RM), Tmm1))

#define shrpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000028C | MXM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0x10000284 | MXM(REG(RG), REG(RG), Tmm1))

#define shrpn_ri(RM, IM)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IM)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(RM), REG(RM), Tmm1))

#define shrpn_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0x1000028C | MXM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0x10000384 | MXM(REG(RG), REG(RG), Tmm1))

/**************************   helper macros (SIMD)   **************************/

/* simd mask */

#define RT_SIMD_MASK_NONE       N       /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       F       /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask
#define SN(rg, lb)  ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END
#define SF(rg, lb)  ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        EMITW(0x10000486 | MXM(REG(RG), REG(RG), TmmQ))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

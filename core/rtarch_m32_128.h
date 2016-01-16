/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_128_H
#define RT_RTARCH_M32_128_H

#include "rtarch_m32.h"

#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32_128.h: Implementation of MIPS32 r5/r6 SIMD-128 instructions.
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
        ((rem) << 16 | (ren) << 11 | (reg) << 6)

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 11 | (reg) << 6)

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0xFF0) << 13)
#define C20(br, dp) EMPTY

#define B21(br) TPxx
#define P21(dp) (0x00000000)
#define C21(br, dp) EMITW(0x34000000 | TDxx << 16 | (0xFFF0 & (dp)))        \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx))

#define B22(br) TPxx
#define P22(dp) (0x00000000)
#define C22(br, dp) EMITW(0x3C000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x34000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFF0 & (dp)))        \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define TmmZ    0x1E  /* w30 */
#define Tmm1    0x1F  /* w31 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, $w0         /* w0 */
#define Xmm1    0x01, 0x00, $w1         /* w1 */
#define Xmm2    0x02, 0x00, $w2         /* w2 */
#define Xmm3    0x03, 0x00, $w3         /* w3 */
#define Xmm4    0x04, 0x00, $w4         /* w4 */
#define Xmm5    0x05, 0x00, $w5         /* w5 */
#define Xmm6    0x06, 0x00, $w6         /* w6 */
#define Xmm7    0x07, 0x00, $w7         /* w7 */
#define Xmm8    0x08, 0x00, $w8         /* w8 */
#define Xmm9    0x09, 0x00, $w9         /* w9 */
#define XmmA    0x0A, 0x00, $w10        /* w10 */
#define XmmB    0x0B, 0x00, $w11        /* w11 */
#define XmmC    0x0C, 0x00, $w12        /* w12 */
#define XmmD    0x0D, 0x00, $w13        /* w13 */
#define XmmE    0x0E, 0x00, $w14        /* w14 */
#define XmmF    0x0F, 0x00, $w15        /* w15 */

/******************************************************************************/
/**********************************   MSA   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        EMITW(0x78BE0019 | MXM(REG(RG), REG(RM), 0x00))

#define movpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

#define movpx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000027 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

#define adrpx_ld(RG, RM, DP) /* RG is a core reg, DP is SIMD-aligned */     \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x00000021 | MXM(REG(RG), MOD(RM), TDxx))

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITW(0x7800001E | MXM(REG(RG), REG(RG), REG(RM)))                  \

#define andpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7800001E | MXM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITW(0x78C0001E | MXM(REG(RG), REG(RM), TmmZ))                     \

#define annpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x4E601C00 | MXM(REG(RG), Tmm1,    TmmZ))

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITW(0x7820001E | MXM(REG(RG), REG(RG), REG(RM)))                  \

#define orrpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7820001E | MXM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITW(0x7860001E | MXM(REG(RG), REG(RG), REG(RM)))                  \

#define xorpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7860001E | MXM(REG(RG), REG(RG), Tmm1))

/**************   packed single precision floating point (SIMD)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITW(0x7800001B | MXM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7800001B | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITW(0x7840001B | MXM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7840001B | MXM(REG(RG), REG(RG), Tmm1))

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITW(0x7880001B | MXM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7880001B | MXM(REG(RG), REG(RG), Tmm1))

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITW(0x78C0001B | MXM(REG(RG), REG(RG), REG(RM)))

#define divps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78C0001B | MXM(REG(RG), REG(RG), Tmm1))

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITW(0x7B26001E | MXM(REG(RG), REG(RM), 0x00))

#define sqrps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B26001E | MXM(REG(RG), Tmm1,    0x00))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        EMITW(0x7B2A001E | MXM(REG(RG), REG(RM), 0x00))

#define rcsps_rr(RG, RM) /* destroys RM */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq */

#define rseps_rr(RG, RM)                                                    \
        EMITW(0x7B28001E | MXM(REG(RG), REG(RM), 0x00))

#define rssps_rr(RG, RM) /* destroys RM */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITW(0x7B00001B | MXM(REG(RG), REG(RG), REG(RM)))                  \

#define minps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B00001B | MXM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITW(0x7B80001B | MXM(REG(RG), REG(RG), REG(RM)))                  \

#define maxps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B80001B | MXM(REG(RG), REG(RG), Tmm1))

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITW(0x7880001A | MXM(REG(RG), REG(RG), REG(RM)))

#define ceqps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7880001A | MXM(REG(RG), REG(RG), Tmm1))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0x78C0001C | MXM(REG(RG), REG(RG), REG(RM)))

#define cneps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78C0001C | MXM(REG(RG), REG(RG), Tmm1))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0x7900001A | MXM(REG(RG), REG(RG), REG(RM)))

#define cltps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7900001A | MXM(REG(RG), REG(RG), Tmm1))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0x7980001A | MXM(REG(RG), REG(RG), REG(RM)))

#define cleps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7980001A | MXM(REG(RG), REG(RG), Tmm1))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0x7900001A | MXM(REG(RG), REG(RM), REG(RG)))

#define cgtps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7900001A | MXM(REG(RG), Tmm1,    REG(RG)))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0x7980001A | MXM(REG(RG), REG(RM), REG(RG)))

#define cgeps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7980001A | MXM(REG(RG), Tmm1,    REG(RG)))

/**************************   packed integer (SIMD)   *************************/

/* cvz
 * rounding mode is encoded directly    (can be used in FCTRL blocks) */

#define rnzps_rr(RG, RM)     /* round towards zero */                       \
        cvzps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define cvzps_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0x7B22001E | MXM(REG(RG), REG(RM), 0x00))                     \

#define cvzps_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B22001E | MXM(REG(RG), Tmm1,    0x00))

/* cvt
 * rounding mode comes from fp control register (set in FCTRL blocks) */

#define rndps_rr(RG, RM)                                                    \
        EMITW(0x7B2A001E | MXM(REG(RG), REG(RM), 0x00))                     \

#define cvtps_rr(RG, RM)                                                    \
        EMITW(0x7B38001E | MXM(REG(RG), REG(RM), 0x00))                     \

#define cvtps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B38001E | MXM(REG(RG), Tmm1,    0x00))                     \

#define cvtpn_rr(RG, RM)                                                    \
        EMITW(0x7B3C001E | MXM(REG(RG), REG(RM), 0x00))

#define cvtpn_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B3C001E | MXM(REG(RG), Tmm1,    0x00))

/* cvn
 * rounding mode is encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round to nearest */                         \
        cvtpn_rr(W(RG), W(RM))

#define cvnpn_ld(RG, RM, DP) /* round to nearest */                         \
        cvtpn_ld(W(RG), W(RM), W(DP))

/* add */

#define addpx_rr(RG, RM)                                                    \
        EMITW(0x7840000E | MXM(REG(RG), REG(RG), REG(RM)))

#define addpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7840000E | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subpx_rr(RG, RM)                                                    \
        EMITW(0x78C0000E | MXM(REG(RG), REG(RG), REG(RM)))

#define subpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78C0000E | MXM(REG(RG), REG(RG), Tmm1))

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        EMITW(0x78400009 | MXM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x1F & VAL(IM)) << 16)

#define shlpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7B02001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x7840000D | MXM(REG(RG), REG(RG), Tmm1))

/* shr */

#define shrpx_ri(RM, IM)                                                    \
        EMITW(0x79400009 | MXM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x1F & VAL(IM)) << 16)

#define shrpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7B02001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x7940000D | MXM(REG(RG), REG(RG), Tmm1))

#define shrpn_ri(RM, IM)                                                    \
        EMITW(0x78C00009 | MXM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x1F & VAL(IM)) << 16)

#define shrpn_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(RM), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7B02001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x78C0000D | MXM(REG(RG), REG(RG), Tmm1))

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * requires MSA-enabled compiler */

#define RT_SIMD_MASK_NONE       N       /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       F       /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask
#define SN(rg, lb)  ASM_BEG ASM_OP2( bz.v, rg, lb) ASM_END
#define SF(rg, lb)  ASM_BEG ASM_OP2(bnz.w, rg, lb) ASM_END

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        AUW(EMPTY, EMPTY, EMPTY, SIB(RG), lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

/* simd mode
 * set in FCTRL blocks (cannot be nested) */

#define RT_SIMD_MODE_ROUNDN     0x00    /* round to nearest */
#define RT_SIMD_MODE_ROUNDM     0x03    /* round towards minus infinity */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards plus  infinity */
#define RT_SIMD_MODE_ROUNDZ     0x01    /* round towards zero */

#define fpscr_ld(RG) /* not portable, do not use outside */                 \
        EMITW(0x783E0019 | MXM(0x01,    REG(RG), 0x00))

#define fpscr_st(RG) /* not portable, do not use outside */                 \
        EMITW(0x787E0019 | MXM(REG(RG), 0x01,    0x00))

#define FCTRL_ENTER(mode) /* assumes default mode (ROUNDN) upon entry */    \
        EMITW(0x34000000 | TIxx << 16 | RT_SIMD_MODE_##mode)                \
        EMITW(0x783E0019 | MXM(0x01,    TIxx,    0x00))

#define FCTRL_LEAVE(mode) /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0x783E0019 | MXM(0x01,    0x00,    0x00))

/* cvr
 * rounding mode is encoded directly (cannot be used in FCTRL blocks) */

#define rnrps_rr(RG, RM, mode)                                              \
        cvrps_rr(W(RG), W(RM), mode)                                        \
        cvnpn_rr(W(RG), W(RG))

#define cvrps_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

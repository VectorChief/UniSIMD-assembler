/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_128_H
#define RT_RTARCH_M32_128_H

#include "rtarch_m32.h"

#define RT_SIMD_REGS        16
#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la
#undef  EMITX
#define EMITX(w) EMITW(w)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32_128.h: Implementation of MIPS32 r5/r6 MSA instructions.
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
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
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

#define Xmm0    0x00, $w0,  EMPTY       /* w0 */
#define Xmm1    0x01, $w1,  EMPTY       /* w1 */
#define Xmm2    0x02, $w2,  EMPTY       /* w2 */
#define Xmm3    0x03, $w3,  EMPTY       /* w3 */
#define Xmm4    0x04, $w4,  EMPTY       /* w4 */
#define Xmm5    0x05, $w5,  EMPTY       /* w5 */
#define Xmm6    0x06, $w6,  EMPTY       /* w6 */
#define Xmm7    0x07, $w7,  EMPTY       /* w7 */
#define Xmm8    0x08, $w8,  EMPTY       /* w8 */
#define Xmm9    0x09, $w9,  EMPTY       /* w9 */
#define XmmA    0x0A, $w10, EMPTY       /* w10 */
#define XmmB    0x0B, $w11, EMPTY       /* w11 */
#define XmmC    0x0C, $w12, EMPTY       /* w12 */
#define XmmD    0x0D, $w13, EMPTY       /* w13 */
#define XmmE    0x0E, $w14, EMPTY       /* w14 */
#define XmmF    0x0F, $w15, EMPTY       /* w15 */

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

#define adrpx_ld(RG, RM, DP) /* RG is a BASE reg, DP is SIMD-aligned */     \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C3(DP), EMPTY2)   \
        EMITW(0x00000021 | MRM(REG(RG), MOD(RM), TDxx))

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movpx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_st(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        EMITW(0x78000027 | MXM(TmmZ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        EMITW(0x78000027 | MXM(Tmm1,    Teax,    0x00))

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movpx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movpx_ld(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        EMITW(0x78000023 | MXM(TmmZ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        EMITW(0x78000023 | MXM(Tmm1,    Teax,    0x00))

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITW(0x7800001E | MXM(REG(RG), REG(RG), REG(RM)))

#define andpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7800001E | MXM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITW(0x78C0001E | MXM(REG(RG), REG(RM), TmmZ))

#define annpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x78C0001E | MXM(REG(RG), Tmm1,    TmmZ))

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITW(0x7820001E | MXM(REG(RG), REG(RG), REG(RM)))

#define orrpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7820001E | MXM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITW(0x7860001E | MXM(REG(RG), REG(RG), REG(RM)))

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

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rceps_rr(RG, RM)                                                    \
        EMITW(0x7B2A001E | MXM(REG(RG), REG(RM), 0x00))

#define rcsps_rr(RG, RM) /* destroys RM */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rseps_rr(RG, RM)                                                    \
        EMITW(0x7B28001E | MXM(REG(RG), REG(RM), 0x00))

#define rssps_rr(RG, RM) /* destroys RM */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITW(0x7B00001B | MXM(REG(RG), REG(RG), REG(RM)))

#define minps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B00001B | MXM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITW(0x7B80001B | MXM(REG(RG), REG(RG), REG(RM)))

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

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzps_rr(RG, RM)     /* round towards zero */                       \
        cvzps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnzps_ld(RG, RM, DP) /* round towards zero */                       \
        cvzps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvzps_rr(RG, RM)     /* round towards zero */                       \
        EMITW(0x7B22001E | MXM(REG(RG), REG(RM), 0x00))

#define cvzps_ld(RG, RM, DP) /* round towards zero */                       \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B22001E | MXM(REG(RG), Tmm1,    0x00))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpps_rr(RG, RM)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rnpps_ld(RG, RM, DP) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndps_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cvpps_rr(RG, RM)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpps_ld(RG, RM, DP) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtps_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmps_rr(RG, RM)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rnmps_ld(RG, RM, DP) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndps_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cvmps_rr(RG, RM)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmps_ld(RG, RM, DP) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtps_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnps_rr(RG, RM)     /* round towards near */                       \
        rndps_rr(W(RG), W(RM))

#define rnnps_ld(RG, RM, DP) /* round towards near */                       \
        rndps_ld(W(RG), W(RM), W(DP))

#define cvnps_rr(RG, RM)     /* round towards near */                       \
        cvtps_rr(W(RG), W(RM))

#define cvnps_ld(RG, RM, DP) /* round towards near */                       \
        cvtps_ld(W(RG), W(RM), W(DP))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round towards near */                       \
        cvtpn_rr(W(RG), W(RM))

#define cvnpn_ld(RG, RM, DP) /* round towards near */                       \
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

/* simd mask (requires MSA-aware compiler)
 * only use first 8 SIMD registers (Xmm0 - Xmm7) as SIMD masks
 * for "potential" future compatibility with wider SIMD (AVX3),
 * which may also require additional macros for SIMD mask ops */

#define RT_SIMD_MASK_NONE       N       /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       F       /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask
#define SN(rg, lb)  ASM_BEG ASM_OP2( bz.v, rg, lb) ASM_END
#define SF(rg, lb)  ASM_BEG ASM_OP2(bnz.w, rg, lb) ASM_END

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        AUW(EMPTY, EMPTY, EMPTY, MOD(RG), lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x03    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x01    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x05    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x05    /* round towards zero */

#define fpscr_ld(RG) /* not portable, do not use outside */                 \
        EMITW(0x783E0019 | MXM(0x01,    REG(RG), 0x00))

#define fpscr_st(RG) /* not portable, do not use outside */                 \
        EMITW(0x787E0019 | MXM(REG(RG), 0x01,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0x34000000 | TNxx << 21 | TIxx << 16 |                        \
                           (RT_SIMD_MODE_##mode&3))                         \
        EMITW(0x783E0019 | MXM(0x01,    TIxx,    0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0x783E0019 | MXM(0x01,    TNxx,    0x00))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0x783E0019 | MXM(0x01, TNxx+(RT_SIMD_MODE_##mode&3), 0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0x783E0019 | MXM(0x01,    TNxx,    0x00))

#endif /* RT_SIMD_FAST_FCTRL */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndps_rr(RG, RM)                                                    \
        EMITW(0x7B2C001E | MXM(REG(RG), REG(RM), 0x00))

#define rndps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B2C001E | MXM(REG(RG), Tmm1,    0x00))

#define cvtps_rr(RG, RM)                                                    \
        EMITW(0x7B38001E | MXM(REG(RG), REG(RM), 0x00))

#define cvtps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B38001E | MXM(REG(RG), Tmm1,    0x00))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtpn_rr(RG, RM)                                                    \
        EMITW(0x7B3C001E | MXM(REG(RG), REG(RM), 0x00))

#define cvtpn_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0x7B3C001E | MXM(REG(RG), Tmm1,    0x00))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrps_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#define cvrps_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtps_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

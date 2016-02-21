/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_128_H
#define RT_RTARCH_ARM_128_H

#include "rtarch_arm.h"

#define RT_SIMD_REGS        8
#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_arm_128.h: Implementation of ARMv7/8:AArch32 NEON-128 instructions.
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
        (((rem) & 0x0F) <<  0 | ((rem) & 0x10) <<  1 |                      \
         ((ren) & 0x0F) << 16 | ((ren) & 0x10) <<  3 |                      \
         ((reg) & 0x0F) << 12 | ((reg) & 0x10) << 18 )

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (bxx(brm) << 16 | (reg) << 12 | pxx(vdp))

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x02000E00 | ((dp) >> 4 & 0xFF))
#define C20(br, dp) EMPTY

#define B21(br) (br)
#define P21(dp) (0x00000000 | TDxx)
#define C21(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFF0 & (dp)))

#define B22(br) (br)
#define P22(dp) (0x00000000 | TDxx)
#define C22(br, dp) EMITW(0xE3000000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0xF0000 & (dp) <<  4) | (0xFF0 & (dp)))        \
                    EMITW(0xE3400000 | MRM(TDxx,    0x00,    0x00) |        \
                            (0x70000 & (dp) >> 12) | (0xFFF & (dp) >> 16))

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tmm0    0x00  /* q0, for integer div VFP fallback */
#define Tmm1    0x10  /* q8 */
#define Tmm2    0x12  /* q9 */
#define Tmm3    0x14  /* q10 */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* q0 */
#define Xmm1    0x02, 0x00, EMPTY       /* q1 */
#define Xmm2    0x04, 0x00, EMPTY       /* q2 */
#define Xmm3    0x06, 0x00, EMPTY       /* q3 */
#define Xmm4    0x08, 0x00, EMPTY       /* q4 */
#define Xmm5    0x0A, 0x00, EMPTY       /* q5 */
#define Xmm6    0x0C, 0x00, EMPTY       /* q6 */
#define Xmm7    0x0E, 0x00, EMPTY       /* q7 */

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/**************************   packed generic (NEON)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MXM(REG(RG), REG(RM), REG(RM)))

#define movpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(REG(RG), TPxx,    0x00))

#define movpx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4000AAF | MXM(REG(RG), TPxx,    0x00))

#define adrpx_ld(RG, RM, DP) /* RG is a core reg, DP is SIMD-aligned */     \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(REG(RG), MOD(RM), VAL(DP), B2(DP), P2(DP)))

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITW(0xF2000150 | MXM(REG(RG), REG(RG), REG(RM)))

#define andpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000150 | MXM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITW(0xF2100150 | MXM(REG(RG), REG(RM), REG(RG)))

#define annpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2100150 | MXM(REG(RG), Tmm1,    REG(RG)))

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MXM(REG(RG), REG(RG), REG(RM)))

#define orrpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200150 | MXM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITW(0xF3000150 | MXM(REG(RG), REG(RG), REG(RM)))

#define xorpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000150 | MXM(REG(RG), REG(RG), Tmm1))

/**************   packed single precision floating point (NEON)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITW(0xF2000D40 | MXM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000D40 | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITW(0xF2200D40 | MXM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200D40 | MXM(REG(RG), REG(RG), Tmm1))

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITW(0xF3000D50 | MXM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000D50 | MXM(REG(RG), REG(RG), Tmm1))

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITW(0xF3BB0540 | MXM(Tmm1,    0x00,    REG(RM))) /* estimate */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    REG(RM))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    REG(RM))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(RG), REG(RG), Tmm1))

#define divps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB0540 | MXM(Tmm1,    0x00,    Tmm3))    /* estimate */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    Tmm3))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    Tmm3))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(RG), REG(RG), Tmm1))

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xF3BB05C0 | MXM(Tmm1,    0x00,    REG(RM))) /* estimate */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    REG(RM))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    REG(RM))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(RG), REG(RM), Tmm1))

#define sqrps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB05C0 | MXM(Tmm1,    0x00,    Tmm3))    /* estimate */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    Tmm3))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    Tmm3))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(RG), Tmm3,    Tmm1))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        EMITW(0xF3BB0540 | MXM(REG(RG), 0x00,    REG(RM)))

#define rcsps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF2000F50 | MXM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF3000D50 | MXM(REG(RG), REG(RG), REG(RM)))

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq */

#define rseps_rr(RG, RM)                                                    \
        EMITW(0xF3BB05C0 | MXM(REG(RG), 0x00,    REG(RM)))

#define rssps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF3000D50 | MXM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF2200F50 | MXM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF3000D50 | MXM(REG(RG), REG(RG), REG(RM)))

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITW(0xF2200F40 | MXM(REG(RG), REG(RG), REG(RM)))

#define minps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200F40 | MXM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITW(0xF2000F40 | MXM(REG(RG), REG(RG), REG(RM)))

#define maxps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000F40 | MXM(REG(RG), REG(RG), Tmm1))

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MXM(REG(RG), REG(RG), REG(RM)))

#define ceqps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MXM(REG(RG), REG(RG), Tmm1))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MXM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF3B005C0 | MXM(REG(RG), 0x00,    REG(RG)))

#define cneps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MXM(REG(RG), REG(RG), Tmm1))                     \
        EMITW(0xF3B005C0 | MXM(REG(RG), 0x00,    REG(RG)))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MXM(REG(RG), REG(RM), REG(RG)))

#define cltps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MXM(REG(RG), Tmm1,    REG(RG)))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MXM(REG(RG), REG(RM), REG(RG)))

#define cleps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MXM(REG(RG), Tmm1,    REG(RG)))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MXM(REG(RG), REG(RG), REG(RM)))

#define cgtps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MXM(REG(RG), REG(RG), Tmm1))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MXM(REG(RG), REG(RG), REG(RM)))

#define cgeps_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MXM(REG(RG), REG(RG), Tmm1))

/**************************   packed integer (NEON)   *************************/

/* cvz
 * rounding mode is encoded directly */

#define rnzps_rr(RG, RM)     /* round towards zero  (fp-to-signed-int) */   \
        cvzps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnzps_ld(RG, RM, DP) /* round towards zero  (fp-to-signed-int) */   \
        cvzps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvzps_rr(RG, RM)     /* round towards zero  (fp-to-signed-int) */   \
        EMITW(0xF3BB0740 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvzps_ld(RG, RM, DP) /* round towards zero  (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0740 | MXM(REG(RG), 0x00,    Tmm1))

#if (RT_128 < 2)

/* cvp
 * rounding mode is encoded directly */

#define fpurp_xx()       /* not portable, do not use outside */             \
        EMITW(0xE3A00501 | MRM(TIxx,    0x00,    0x00))                     \
        EMITW(0xEEE10A10 | MRM(TIxx,    0x00,    0x00))

#define rnpps_rr(RG, RM)     /* round towards +inf  (fp-to-signed-int) */   \
        cvpps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnpps_ld(RG, RM, DP) /* round towards +inf  (fp-to-signed-int) */   \
        cvpps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvpps_rr(RG, RM)     /* round towards +inf  (fp-to-signed-int) */   \
        fpurp_xx()                                                          \
        cvnps_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvpps_ld(RG, RM, DP) /* round towards +inf  (fp-to-signed-int) */   \
        fpurp_xx()                                                          \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvm
 * rounding mode is encoded directly */

#define fpurm_xx()       /* not portable, do not use outside */             \
        EMITW(0xE3A00502 | MRM(TIxx,    0x00,    0x00))                     \
        EMITW(0xEEE10A10 | MRM(TIxx,    0x00,    0x00))

#define rnmps_rr(RG, RM)     /* round towards -inf  (fp-to-signed-int) */   \
        cvmps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnmps_ld(RG, RM, DP) /* round towards -inf  (fp-to-signed-int) */   \
        cvmps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvmps_rr(RG, RM)     /* round towards -inf  (fp-to-signed-int) */   \
        fpurm_xx()                                                          \
        cvnps_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvmps_ld(RG, RM, DP) /* round towards -inf  (fp-to-signed-int) */   \
        fpurm_xx()                                                          \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvn
 * rounding mode is encoded directly */

#define fpurn_xx()       /* not portable, do not use outside */             \
        EMITW(0xEEE10A10 | MRM(TZxx,    0x00,    0x00))

#define rnnps_rr(RG, RM)     /* round to nearest    (fp-to-signed-int) */   \
        cvnps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnnps_ld(RG, RM, DP) /* round to nearest    (fp-to-signed-int) */   \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvnps_rr(RG, RM)     /* fallback to VFP for float-to-integer cvt */ \
        EMITW(0xEEBD0A40 | MXM(REG(RG)+0, 0x00,  REG(RM)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MXM(REG(RG)+0, 0x00,  REG(RM)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MXM(REG(RG)+1, 0x00,  REG(RM)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MXM(REG(RG)+1, 0x00,  REG(RM)+1)) /* modes */

#define cvnps_ld(RG, RM, DP) /* fallback to VFP for float-to-integer cvt */ \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(REG(RG), TPxx,    0x00))                     \
        EMITW(0xEEBD0A40 | MXM(REG(RG)+0, 0x00,  REG(RG)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MXM(REG(RG)+0, 0x00,  REG(RG)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MXM(REG(RG)+1, 0x00,  REG(RG)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MXM(REG(RG)+1, 0x00,  REG(RG)+1)) /* modes */

#else /* RT_128 >= 2 */

/* cvp
 * rounding mode is encoded directly */

#define rnpps_rr(RG, RM)     /* round towards +inf  (fp-to-signed-int) */   \
        cvpps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnpps_ld(RG, RM, DP) /* round towards +inf  (fp-to-signed-int) */   \
        cvpps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvpps_rr(RG, RM)     /* round towards +inf  (fp-to-signed-int) */   \
        EMITW(0xF3BB0240 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvpps_ld(RG, RM, DP) /* round towards +inf  (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0240 | MXM(REG(RG), 0x00,    Tmm1))

/* cvm
 * rounding mode is encoded directly */

#define rnmps_rr(RG, RM)     /* round towards -inf  (fp-to-signed-int) */   \
        cvmps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnmps_ld(RG, RM, DP) /* round towards -inf  (fp-to-signed-int) */   \
        cvmps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvmps_rr(RG, RM)     /* round towards -inf  (fp-to-signed-int) */   \
        EMITW(0xF3BB0340 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvmps_ld(RG, RM, DP) /* round towards -inf  (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0340 | MXM(REG(RG), 0x00,    Tmm1))

/* cvn
 * rounding mode is encoded directly */

#define rnnps_rr(RG, RM)     /* round to nearest    (fp-to-signed-int) */   \
        cvnps_rr(W(RG), W(RM))                                              \
        cvnpn_rr(W(RG), W(RG))

#define rnnps_ld(RG, RM, DP) /* round to nearest    (fp-to-signed-int) */   \
        cvnps_ld(W(RG), W(RM), W(DP))                                       \
        cvnpn_rr(W(RG), W(RG))

#define cvnps_rr(RG, RM)     /* round to nearest    (fp-to-signed-int) */   \
        EMITW(0xF3BB0140 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvnps_ld(RG, RM, DP) /* round to nearest    (fp-to-signed-int) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0140 | MXM(REG(RG), 0x00,    Tmm1))

#endif /* RT_128 >= 2 */

/* cvn
 * rounding mode is encoded directly */

#define cvnpn_rr(RG, RM)     /* round to nearest    (signed-int-to-fp) */   \
        EMITW(0xF3BB0640 | MXM(REG(RG), 0x00,    REG(RM)))

#define cvnpn_ld(RG, RM, DP) /* round to nearest    (signed-int-to-fp) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0640 | MXM(REG(RG), 0x00,    Tmm1))

/* add */

#define addpx_rr(RG, RM)                                                    \
        EMITW(0xF2200840 | MXM(REG(RG), REG(RG), REG(RM)))

#define addpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200840 | MXM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subpx_rr(RG, RM)                                                    \
        EMITW(0xF3200840 | MXM(REG(RG), REG(RG), REG(RM)))

#define subpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200840 | MXM(REG(RG), REG(RG), Tmm1))

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        EMITW(0xF2A00550 | MXM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 16)

#define shlpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4A00CBF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200440 | MXM(REG(RG), Tmm1,    REG(RG)))

/* shr */

#define shrpx_ri(RM, IM) /* emit shift-left for zero-immediate args */      \
        EMITW(0xF2A00050 | MXM(REG(RM), 0x00,    REG(RM)) |                 \
        (+(VAL(IM) == 0) & 0x00000500) | (+(VAL(IM) != 0) & 0x01000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IM)) << 16)

#define shrpx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4A00CBF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF3200440 | MXM(REG(RG), Tmm1,    REG(RG)))

#define shrpn_ri(RM, IM) /* emit shift-left for zero-immediate args */      \
        EMITW(0xF2A00050 | MXM(REG(RM), 0x00,    REG(RM)) |                 \
        (+(VAL(IM) == 0) & 0x00000500) | (+(VAL(IM) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IM)) << 16)

#define shrpn_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C2(DP), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(RM), VAL(DP), B2(DP), P2(DP)))  \
        EMITW(0xF4A00CBF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF2200440 | MXM(REG(RG), Tmm1,    REG(RG)))

/**************************   helper macros (NEON)   **************************/

/* simd mask */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x01    /*  all satisfy the condition */

#define movms_rr(RG, RM) /* not portable, do not use outside */             \
        EMITW(0xF3B60200 | MXM(Tmm1+0,  0x00,    REG(RM)))                  \
        EMITW(0xF3B20200 | MXM(Tmm1+0,  0x00,    Tmm1))                     \
        EMITW(0xEE100B10 | MXM(REG(RG), Tmm1+0,  0x00))

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        movms_rr(Reax, W(RG))                                               \
        addxx_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        jezxx_lb(lb)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_ARM_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

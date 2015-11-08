/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_MPE_H
#define RT_RTARCH_ARM_MPE_H

#include "rtarch_arm.h"

#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_arm_mpe.h: Implementation of ARM SIMD instructions.
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
 * cmdp*_mr - applies [cmd] to [p]acked: [m]emory   from [r]egister
 * cmdp*_st - applies [cmd] to [p]acked: as above (arg list as cmdp*_ld)
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

#define MTM(reg, ren, rem)                                                  \
        (((rem) & 0x0F) <<  0 | ((rem) & 0x10) <<  1 |                      \
         ((ren) & 0x0F) << 16 | ((ren) & 0x10) <<  3 |                      \
         ((reg) & 0x0F) << 12 | ((reg) & 0x10) << 18 )

/* registers    REG */

#define Tmm0    0x00                    /* q0, for integer div VFP fallback */
#define Tmm1    0x10                    /* q8 */
#define Tmm2    0x12                    /* q9 */
#define Tmm3    0x14                    /* q10 */

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
        EMITW(0xF2200150 | MTM(REG(RG), REG(RM), REG(RM)))

#define movpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(REG(RG), TPxx,    0x00))

#define movpx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4000AAF | MTM(REG(RG), TPxx,    0x00))

#define adrpx_ld(RG, RM, DP) /* RG is a core reg, DP is SIMD-aligned */     \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MTM(REG(RG), MOD(RM), 0x00) | TYP(DP))

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITW(0xF2000150 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define andpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000150 | MTM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITW(0xF2100150 | MTM(REG(RG), REG(RM), REG(RG)))                  \

#define annpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2100150 | MTM(REG(RG), Tmm1,    REG(RG)))

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITW(0xF2200150 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define orrpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200150 | MTM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITW(0xF3000150 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define xorpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000150 | MTM(REG(RG), REG(RG), Tmm1))

/**************   packed single precision floating point (NEON)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITW(0xF2000D40 | MTM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000D40 | MTM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITW(0xF2200D40 | MTM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200D40 | MTM(REG(RG), REG(RG), Tmm1))

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), Tmm1))

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITW(0xF3BB0540 | MTM(Tmm1,    0x00,    REG(RM))) /* estimate */   \
        EMITW(0xF2000F50 | MTM(Tmm2,    Tmm1,    REG(RM))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MTM(Tmm2,    Tmm1,    REG(RM))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), Tmm1))

#define divps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB0540 | MTM(Tmm1,    0x00,    Tmm3))    /* estimate */   \
        EMITW(0xF2000F50 | MTM(Tmm2,    Tmm1,    Tmm3))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MTM(Tmm2,    Tmm1,    Tmm3))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), Tmm1))

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITW(0xF3BB05C0 | MTM(Tmm1,    0x00,    REG(RM))) /* estimate */   \
        EMITW(0xF3000D50 | MTM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MTM(Tmm2,    Tmm2,    REG(RM))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MTM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MTM(Tmm2,    Tmm2,    REG(RM))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RM), Tmm1))

#define sqrps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB05C0 | MTM(Tmm1,    0x00,    Tmm3))    /* estimate */   \
        EMITW(0xF3000D50 | MTM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MTM(Tmm2,    Tmm2,    Tmm3))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MTM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MTM(Tmm2,    Tmm2,    Tmm3))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MTM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MTM(REG(RG), Tmm3,    Tmm1))

/* cbr */

        /* cbe, cbs, cbr defined in rtarch.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        EMITW(0xF3BB0540 | MTM(REG(RG), 0x00,    REG(RM)))

#define rcsps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF2000F50 | MTM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), REG(RM)))

        /* rcp defined in rtarch.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq */

#define rseps_rr(RG, RM)                                                    \
        EMITW(0xF3BB05C0 | MTM(REG(RG), 0x00,    REG(RM)))

#define rssps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0xF3000D50 | MTM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF2200F50 | MTM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0xF3000D50 | MTM(REG(RG), REG(RG), REG(RM)))

        /* rsq defined in rtarch.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITW(0xF2200F40 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define minps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200F40 | MTM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITW(0xF2000F40 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define maxps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000F40 | MTM(REG(RG), REG(RG), Tmm1))

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MTM(REG(RG), REG(RG), REG(RM)))

#define ceqps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MTM(REG(RG), REG(RG), Tmm1))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0xF2000E40 | MTM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF3B005C0 | MTM(REG(RG), 0x00,    REG(RG)))

#define cneps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MTM(REG(RG), REG(RG), Tmm1))                     \
        EMITW(0xF3B005C0 | MTM(REG(RG), 0x00,    REG(RG)))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MTM(REG(RG), REG(RM), REG(RG)))

#define cltps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MTM(REG(RG), Tmm1,    REG(RG)))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MTM(REG(RG), REG(RM), REG(RG)))

#define cleps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MTM(REG(RG), Tmm1,    REG(RG)))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0xF3200E40 | MTM(REG(RG), REG(RG), REG(RM)))

#define cgtps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MTM(REG(RG), REG(RG), Tmm1))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0xF3000E40 | MTM(REG(RG), REG(RG), REG(RM)))

#define cgeps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MTM(REG(RG), REG(RG), Tmm1))

/**************************   packed integer (NEON)   *************************/

/* cvt */

#define cvtps_rr(RG, RM)     /* fallback to VFP for float-to-integer cvt */ \
        EMITW(0xEEBD0A40 | MTM(REG(RG)+0, 0x00,  REG(RM)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MTM(REG(RG)+0, 0x00,  REG(RM)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MTM(REG(RG)+1, 0x00,  REG(RM)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MTM(REG(RG)+1, 0x00,  REG(RM)+1)) /* modes */

#define cvtps_ld(RG, RM, DP) /* fallback to VFP for float-to-integer cvt */ \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(REG(RG), TPxx,    0x00))                     \
        EMITW(0xEEBD0A40 | MTM(REG(RG)+0, 0x00,  REG(RG)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MTM(REG(RG)+0, 0x00,  REG(RG)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MTM(REG(RG)+1, 0x00,  REG(RG)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MTM(REG(RG)+1, 0x00,  REG(RG)+1)) /* modes */

#define cvtpn_rr(RG, RM)                                                    \
        EMITW(0xF3BB0640 | MRM(REG(RG), 0x00,    REG(RM)))

#define cvtpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0640 | MTM(REG(RG), 0x00,    Tmm1))

/* add */

#define addpx_rr(RG, RM)                                                    \
        EMITW(0xF2200840 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define addpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200840 | MTM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subpx_rr(RG, RM)                                                    \
        EMITW(0xF3200840 | MTM(REG(RG), REG(RG), REG(RM)))                  \

#define subpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4200AAF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200840 | MTM(REG(RG), REG(RG), Tmm1))

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        EMITW(0xF2A00550 | MTM(REG(RM), 0x00,    REG(RM)) |                 \
                                                 (0x1F & VAL(IM)) << 16)

#define shlpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4A00CBF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200440 | MTM(REG(RG), Tmm1,    REG(RG)))

/* shr */

#define shrpx_ri(RM, IM) /* few extra ops to support shifts by zero */      \
        EMITW(0xE3A00000 | MRM(TIxx,    0x00,    0x00) |(VAL(IM) & 0x1F))   \
        EMITW(0xEEA00B10 | MTM(TIxx,    Tmm1,    0x00))                     \
        EMITW(0xF3B903C0 | MTM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF3200440 | MTM(REG(RM), Tmm1,    REG(RM)))

#define shrpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4A00CBF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MTM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF3200440 | MTM(REG(RG), Tmm1,    REG(RG)))

#define shrpn_ri(RM, IM) /* few extra ops to support shifts by zero */      \
        EMITW(0xE3A00000 | MRM(TIxx,    0x00,    0x00) |(VAL(IM) & 0x1F))   \
        EMITW(0xEEA00B10 | MTM(TIxx,    Tmm1,    0x00))                     \
        EMITW(0xF3B903C0 | MTM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF2200440 | MTM(REG(RM), Tmm1,    REG(RM)))

#define shrpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0xE0800000 | MRM(TPxx,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0xF4A00CBF | MTM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MTM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF2200440 | MTM(REG(RG), Tmm1,    REG(RG)))

/**************************   helper macros (NEON)   **************************/

/* simd mask */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x01    /*  all satisfy the condition */

#define movms_rr(RG, RM) /* not portable, do not use outside */             \
        EMITW(0xF3B60200 | MTM(Tmm1+0,  0x00,    REG(RM)))                  \
        EMITW(0xF3B20200 | MTM(Tmm1+0,  0x00,    Tmm1))                     \
        EMITW(0xEE100B10 | MTM(REG(RG), Tmm1+0,  0x00))

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        movms_rr(Reax, W(RG))                                               \
        addxx_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        cmpxx_ri(Reax, IB(0))                                               \
        jeqxx_lb(lb)

/* simd mode */

#define RT_SIMD_MODE_ROUNDN     0x00    /* round to nearest */
#define RT_SIMD_MODE_ROUNDM     0x02    /* round towards minus infinity */
#define RT_SIMD_MODE_ROUNDP     0x01    /* round towards plus  infinity */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#define fpscr_ld(RG) /* not portable, do not use outside */                 \
        EMITW(0xEEE10A10 | MRM(REG(RG), 0x00,    0x00))

#define fpscr_st(RG) /* not portable, do not use outside */                 \
        EMITW(0xEEF10A10 | MRM(REG(RG), 0x00,    0x00))

#define FCTRL_ENTER(mode) /* destroys Reax */                               \
        fpscr_st(Reax)                                                      \
        movxx_st(Reax, Mebp, inf_FCTRL)                                     \
        orrxx_ri(Reax, IW(RT_SIMD_MODE_##mode << 22))                       \
        fpscr_ld(Reax)

#define FCTRL_LEAVE(mode) /* destroys Reax */                               \
        movxx_ld(Reax, Mebp, inf_FCTRL)                                     \
        fpscr_ld(Reax)

#endif /* RT_RTARCH_ARM_MPE_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

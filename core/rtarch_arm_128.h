/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_128_H
#define RT_RTARCH_ARM_128_H

#include "rtarch_arm.h"

#define RT_SIMD_REGS        8
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
 * rtarch_arm_128.h: Implementation of ARMv7/8:AArch32 fp32 NEON instructions.
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

#define movox_rr(XD, XS)                                                    \
        EMITW(0xF2200150 | MXM(REG(XD), REG(XS), REG(XS)))

#define movox_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))

#define movox_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0xF4000AAF | MXM(REG(XS), TPxx,    0x00))


#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, DS is SIMD-aligned */     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(REG(RD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

/* and */

#define andox_rr(XG, XS)                                                    \
        EMITW(0xF2000150 | MXM(REG(XG), REG(XG), REG(XS)))

#define andox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000150 | MXM(REG(XG), REG(XG), Tmm1))

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        EMITW(0xF2100150 | MXM(REG(XG), REG(XS), REG(XG)))

#define annox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2100150 | MXM(REG(XG), Tmm1,    REG(XG)))

/* orr */

#define orrox_rr(XG, XS)                                                    \
        EMITW(0xF2200150 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200150 | MXM(REG(XG), REG(XG), Tmm1))

/* orn (G = ~G | S) */

#define ornox_rr(XG, XS)                                                    \
        EMITW(0xF2300150 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2300150 | MXM(REG(XG), Tmm1,    REG(XG)))

/* xor */

#define xorox_rr(XG, XS)                                                    \
        EMITW(0xF3000150 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000150 | MXM(REG(XG), REG(XG), Tmm1))

/* not */

#define notox_rx(XG)                                                        \
        EMITW(0xF2B005C0 | MXM(REG(XG), 0x00,    REG(XG)))

/**************   packed single precision floating point (NEON)   *************/

/* neg */

#define negos_rx(XG)                                                        \
        EMITW(0xF2B907C0 | MXM(REG(XG), 0x00,    REG(XG)))

#if (RT_128 < 2) /* vector FMA is available in processors with ASIMDv2 */

/* NOTE: implement later via VFP using scalar double-precision */

#else /* RT_128 >= 2 */

/* fma (G = G + S * T) */

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0xF2000C50 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000C50 | MXM(REG(XG), REG(XS), Tmm1))

/* fms (G = G - S * T) */

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0xF2200C50 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200C50 | MXM(REG(XG), REG(XS), Tmm1))

#endif /* RT_128 >= 2 */

/* add */

#define addos_rr(XG, XS)                                                    \
        EMITW(0xF2000D40 | MXM(REG(XG), REG(XG), REG(XS)))

#define addos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000D40 | MXM(REG(XG), REG(XG), Tmm1))

/* sub */

#define subos_rr(XG, XS)                                                    \
        EMITW(0xF2200D40 | MXM(REG(XG), REG(XG), REG(XS)))

#define subos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200D40 | MXM(REG(XG), REG(XG), Tmm1))

/* mul */

#define mulos_rr(XG, XS)                                                    \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), REG(XS)))

#define mulos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), Tmm1))

/* div */

#if RT_SIMD_COMPAT_DIV != 0

#define divos_rr(XG, XS)                                                    \
        EMITW(0xEE800A00 | MRM(REG(XG)+0, REG(XG)+0, REG(XS)+0))            \
        EMITW(0xEEC00AA0 | MRM(REG(XG)+0, REG(XG)+0, REG(XS)+0))            \
        EMITW(0xEE800A00 | MRM(REG(XG)+1, REG(XG)+1, REG(XS)+1))            \
        EMITW(0xEEC00AA0 | MRM(REG(XG)+1, REG(XG)+1, REG(XS)+1))

#define divos_ld(XG, MS, DS)                                                \
        movox_st(Xmm0, Mebp, inf_SCR01(0))                                  \
        movox_ld(Xmm0, W(MS), W(DS))                                        \
        divos_rr(W(XG), Xmm0)                                               \
        movox_ld(Xmm0, Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_DIV */

#if (RT_128 < 2) /* vector FMA is available in processors with ASIMDv2 */

#define divos_rr(XG, XS)                                                    \
        EMITW(0xF3BB0540 | MXM(Tmm1,    0x00,    REG(XS))) /* estimate */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    REG(XS))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    REG(XS))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    REG(XS))) /* 3rd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    REG(XG), Tmm1))                     \
        EMITW(0xF2200D50 | MXM(REG(XG), REG(XS), Tmm2))    /* residual */   \
        EMITW(0xF2000D50 | MXM(Tmm2,    REG(XG), Tmm1))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), Tmm2,    Tmm2))

#define divos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB0540 | MXM(Tmm1,    0x00,    Tmm3))    /* estimate */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    Tmm3))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    Tmm3))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    Tmm3))    /* 3rd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    REG(XG), Tmm1))                     \
        EMITW(0xF2200D50 | MXM(REG(XG), Tmm3,    Tmm2))    /* residual */   \
        EMITW(0xF2000D50 | MXM(Tmm2,    REG(XG), Tmm1))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), Tmm2,    Tmm2))

#else /* RT_128 >= 2 */

#define divos_rr(XG, XS)                                                    \
        EMITW(0xF3BB0540 | MXM(Tmm1,    0x00,    REG(XS))) /* estimate */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    REG(XS))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    REG(XG), Tmm1))                     \
        EMITW(0xF2200C50 | MXM(REG(XG), REG(XS), Tmm2))    /* residual */   \
        EMITW(0xF2000C50 | MXM(Tmm2,    REG(XG), Tmm1))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), Tmm2,    Tmm2))

#define divos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB0540 | MXM(Tmm1,    0x00,    Tmm3))    /* estimate */   \
        EMITW(0xF2000F50 | MXM(Tmm2,    Tmm1,    Tmm3))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    REG(XG), Tmm1))                     \
        EMITW(0xF2200C50 | MXM(REG(XG), Tmm3,    Tmm2))    /* residual */   \
        EMITW(0xF2000C50 | MXM(Tmm2,    REG(XG), Tmm1))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), Tmm2,    Tmm2))

#endif /* RT_128 >= 2 */

#endif /* RT_SIMD_COMPAT_DIV */

/* sqr */

#if RT_SIMD_COMPAT_SQR != 0

#define sqros_rr(XD, XS)                                                    \
        EMITW(0xEEB10AC0 | MRM(REG(XD)+0, 0x00, REG(XS)+0))                 \
        EMITW(0xEEF10AE0 | MRM(REG(XD)+0, 0x00, REG(XS)+0))                 \
        EMITW(0xEEB10AC0 | MRM(REG(XD)+1, 0x00, REG(XS)+1))                 \
        EMITW(0xEEF10AE0 | MRM(REG(XD)+1, 0x00, REG(XS)+1))

#define sqros_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        sqros_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SQR */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0xF3BB05C0 | MXM(Tmm1,    0x00,    REG(XS))) /* estimate */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    REG(XS))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    REG(XS))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(XD), REG(XS), Tmm1))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm3,    TPxx,    0x00))                     \
        EMITW(0xF3BB05C0 | MXM(Tmm1,    0x00,    Tmm3))    /* estimate */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    Tmm3))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(Tmm2,    Tmm1,    Tmm1))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(Tmm2,    Tmm2,    Tmm3))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(Tmm1,    Tmm1,    Tmm2))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(XD), Tmm3,    Tmm1))

#endif /* RT_SIMD_COMPAT_SQR */

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XD, XS)                                                    \
        EMITW(0xF3BB0540 | MXM(REG(XD), 0x00,    REG(XS)))

#define rcsos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0xF2000F50 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseos_rr(XD, XS)                                                    \
        EMITW(0xF3BB05C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rssos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0xF3000D50 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0xF2200F50 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minos_rr(XG, XS)                                                    \
        EMITW(0xF2200F40 | MXM(REG(XG), REG(XG), REG(XS)))

#define minos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200F40 | MXM(REG(XG), REG(XG), Tmm1))

/* max */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0xF2000F40 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000F40 | MXM(REG(XG), REG(XG), Tmm1))

/* cmp */

#define ceqos_rr(XG, XS)                                                    \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), Tmm1))

#define cneos_rr(XG, XS)                                                    \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF3B005C0 | MXM(REG(XG), 0x00,    REG(XG)))

#define cneos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), Tmm1))                     \
        EMITW(0xF3B005C0 | MXM(REG(XG), 0x00,    REG(XG)))

#define cltos_rr(XG, XS)                                                    \
        EMITW(0xF3200E40 | MXM(REG(XG), REG(XS), REG(XG)))

#define cltos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MXM(REG(XG), Tmm1,    REG(XG)))

#define cleos_rr(XG, XS)                                                    \
        EMITW(0xF3000E40 | MXM(REG(XG), REG(XS), REG(XG)))

#define cleos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MXM(REG(XG), Tmm1,    REG(XG)))

#define cgtos_rr(XG, XS)                                                    \
        EMITW(0xF3200E40 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MXM(REG(XG), REG(XG), Tmm1))

#define cgeos_rr(XG, XS)                                                    \
        EMITW(0xF3000E40 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgeos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MXM(REG(XG), REG(XG), Tmm1))

/**************************   packed integer (NEON)   *************************/

#if (RT_128 < 4) /* ASIMDv4 is used here for ARMv8:AArch32 processors */

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        cvzos_rr(W(XD), W(XS))                                              \
        cvnon_rr(W(XD), W(XD))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        cvzos_ld(W(XD), W(MS), W(DS))                                       \
        cvnon_rr(W(XD), W(XD))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    Tmm1))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        cvpos_rr(W(XD), W(XS))                                              \
        cvnon_rr(W(XD), W(XD))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpos_ld(W(XD), W(MS), W(DS))                                       \
        cvnon_rr(W(XD), W(XD))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        cvmos_rr(W(XD), W(XS))                                              \
        cvnon_rr(W(XD), W(XD))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmos_ld(W(XD), W(MS), W(DS))                                       \
        cvnon_rr(W(XD), W(XD))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        cvnos_rr(W(XD), W(XS))                                              \
        cvnon_rr(W(XD), W(XD))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        cvnos_ld(W(XD), W(MS), W(DS))                                       \
        cvnon_rr(W(XD), W(XD))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        cvtos_rr(W(XD), W(XS))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        cvtos_ld(W(XD), W(MS), W(DS))

#else /* RT_128 >= 4 */

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF3BA05C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BA05C0 | MXM(REG(XD), 0x00,    Tmm1))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    Tmm1))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF3BA07C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BA07C0 | MXM(REG(XD), 0x00,    Tmm1))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF3BB0240 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0240 | MXM(REG(XD), 0x00,    Tmm1))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF3BA06C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BA06C0 | MXM(REG(XD), 0x00,    Tmm1))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF3BB0340 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0340 | MXM(REG(XD), 0x00,    Tmm1))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF3BA0440 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BA0440 | MXM(REG(XD), 0x00,    Tmm1))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF3BB0140 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0140 | MXM(REG(XD), 0x00,    Tmm1))

#endif /* RT_128 >= 4 */

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF3BB0640 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3BB0640 | MXM(REG(XD), 0x00,    Tmm1))

/* add */

#define addox_rr(XG, XS)                                                    \
        EMITW(0xF2200840 | MXM(REG(XG), REG(XG), REG(XS)))

#define addox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF2200840 | MXM(REG(XG), REG(XG), Tmm1))

/* sub */

#define subox_rr(XG, XS)                                                    \
        EMITW(0xF3200840 | MXM(REG(XG), REG(XG), REG(XS)))

#define subox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200840 | MXM(REG(XG), REG(XG), Tmm1))

/* shl */

#define shlox_ri(XG, IS)                                                    \
        EMITW(0xF2A00550 | MXM(REG(XG), 0x00,    REG(XG)) |                 \
                                                 (0x1F & VAL(IS)) << 16)

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4A00CBF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3200440 | MXM(REG(XG), Tmm1,    REG(XG)))

/* shr */

#define shrox_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0xF2A00050 | MXM(REG(XG), 0x00,    REG(XG)) |                 \
        (+(VAL(IS) == 0) & 0x00000500) | (+(VAL(IS) != 0) & 0x01000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4A00CBF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF3200440 | MXM(REG(XG), Tmm1,    REG(XG)))

#define shron_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0xF2A00050 | MXM(REG(XG), 0x00,    REG(XG)) |                 \
        (+(VAL(IS) == 0) & 0x00000500) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4A00CBF | MXM(Tmm1,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(Tmm1,    0x00,    Tmm1))                     \
        EMITW(0xF2200440 | MXM(REG(XG), Tmm1,    REG(XG)))

/**************************   helper macros (NEON)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x01    /*  all satisfy the condition */

#define movms_rr(RD, XS) /* not portable, do not use outside */             \
        EMITW(0xF3B60200 | MXM(Tmm1+0,  0x00,    REG(XS)))                  \
        EMITW(0xF3B20200 | MXM(Tmm1+0,  0x00,    Tmm1))                     \
        EMITW(0xEE100B10 | MXM(REG(RD), Tmm1+0,  0x00))

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
        EMITW(0xEEE10A10 | MRM(REG(RS), 0x00,    0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xEEF10A10 | MRM(REG(RD), 0x00,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xE3A00500 | MRM(TIxx,    0x00,    0x00) |                    \
                           RT_SIMD_MODE_##mode)                             \
        EMITW(0xEEE10A10 | MRM(TIxx,    0x00,    0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xEEE10A10 | MRM(TNxx,    0x00,    0x00))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xEEE10A10 | MRM((RT_SIMD_MODE_##mode&3)*2+8, 0x00, 0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xEEE10A10 | MRM(TNxx,    0x00,    0x00))

#endif /* RT_SIMD_FAST_FCTRL */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4) /* ASIMDv4 is used here for ARMv8:AArch32 processors */

#define rndos_rr(XD, XS)                                                    \
        cvtos_rr(W(XD), W(XS))                                              \
        cvnon_rr(W(XD), W(XD))

#define rndos_ld(XD, MS, DS)                                                \
        cvtos_ld(W(XD), W(MS), W(DS))                                       \
        cvnon_rr(W(XD), W(XD))

#else /* RT_128 >= 4 */

#define rndos_rr(XD, XS)     /* fallback to VFP for float-to-integer rnd */ \
        EMITW(0xEEB60A40 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* due to */   \
        EMITW(0xEEF60A60 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* lack of */  \
        EMITW(0xEEB60A40 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* rounding */ \
        EMITW(0xEEF60A60 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* modes */

#define rndos_ld(XD, MS, DS) /* fallback to VFP for float-to-integer rnd */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))                     \
        EMITW(0xEEB60A40 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* due to */   \
        EMITW(0xEEF60A60 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* lack of */  \
        EMITW(0xEEB60A40 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* rounding */ \
        EMITW(0xEEF60A60 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* modes */

#endif /* RT_128 >= 4 */

#define cvtos_rr(XD, XS)     /* fallback to VFP for float-to-integer cvt */ \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* modes */

#define cvtos_ld(XD, MS, DS) /* fallback to VFP for float-to-integer cvt */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))                     \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* modes */

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)     /* fallback to VFP for integer-to-float cvt */ \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* due to */   \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* lack of */  \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* rounding */ \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* modes */

#define cvton_ld(XD, MS, DS) /* fallback to VFP for integer-to-float cvt */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))                     \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* due to */   \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* lack of */  \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* rounding */ \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* modes */

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4) /* ASIMDv4 is used here for ARMv8:AArch32 processors */

#define rnros_rr(XD, XS, mode)                                              \
        cvros_rr(W(XD), W(XS), mode)                                        \
        cvnon_rr(W(XD), W(XD))

#define cvros_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_128 >= 4 */

#define rnros_rr(XD, XS, mode)                                              \
        cvros_rr(W(XD), W(XS), mode)                                        \
        cvnon_rr(W(XD), W(XD))

#define cvros_rr(XD, XS, mode)                                              \
        EMITW(0xF3BB0040 | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        ((RT_SIMD_MODE_##mode&3)+1 + 3*(((RT_SIMD_MODE_##mode&3)+1) >> 2)) << 8)

#endif /* RT_128 >= 4 */

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
        EMITW(0xF4000AAF | MXM(Tmm1,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4000AAF | MXM(Tmm2,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4000AAF | MXM(Tmm3,    Teax,    0x00))

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
        EMITW(0xF4200AAF | MXM(Tmm1,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4200AAF | MXM(Tmm2,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4200AAF | MXM(Tmm3,    Teax,    0x00))

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_ARM_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

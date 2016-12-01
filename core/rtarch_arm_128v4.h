/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ARM_128V4_H
#define RT_RTARCH_ARM_128V4_H

#include "rtarch_arm.h"

#define RT_SIMD_REGS        8
#define RT_SIMD_ALIGN       16
#define RT_SIMD_WIDTH32     4
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH64     2
#define RT_SIMD_SET64(s, v) s[0]=s[1]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_arm_128v4.h: Implementation of ARMv7/8:AArch32 fp32 NEON instructions.
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

#if defined (RT_128) && (RT_128 != 0)

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

#define TmmC    0x12  /* q9 */
#define TmmD    0x14  /* q10 */
#define TmmE    0x16  /* q11 */
#define TmmF    0x18  /* q12 */

#define Tmm0    0x00  /* q0, internal name for Xmm0 (in mmv, VFP-int-div) */
#define TmmM    0x10  /* q8, temp-reg name for mem-args */

/* register pass-through variator */

#define   V(reg, mod, sib)  ((reg + 0x02) & 0x0F), mod, sib

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

/* adr (D = adr S) */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, MS/DS is SIMD-aligned */  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(REG(RD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

/**************************   packed generic (NEON)   *************************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMITW(0xF2200150 | MXM(REG(XD), REG(XS), REG(XS)))

#define movix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))

#define movix_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0xF4000AAF | MXM(REG(XS), TPxx,    0x00))

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvix_rr(XG, XS)                                                    \
        EMITW(0xF3200150 | MXM(REG(XG), REG(XS), Tmm0))

#define mmvix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3200150 | MXM(REG(XG), TmmM,    Tmm0))

#define mmvix_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3200150 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0xF4000AAF | MXM(TmmM,    TPxx,    0x00))

/* and (G = G & S) */

#define andix_rr(XG, XS)                                                    \
        EMITW(0xF2000150 | MXM(REG(XG), REG(XG), REG(XS)))

#define andix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2000150 | MXM(REG(XG), REG(XG), TmmM))

/* ann (G = ~G & S) */

#define annix_rr(XG, XS)                                                    \
        EMITW(0xF2100150 | MXM(REG(XG), REG(XS), REG(XG)))

#define annix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2100150 | MXM(REG(XG), TmmM,    REG(XG)))

/* orr (G = G | S) */

#define orrix_rr(XG, XS)                                                    \
        EMITW(0xF2200150 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2200150 | MXM(REG(XG), REG(XG), TmmM))

/* orn (G = ~G | S) */

#define ornix_rr(XG, XS)                                                    \
        EMITW(0xF2300150 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2300150 | MXM(REG(XG), TmmM,    REG(XG)))

/* xor (G = G ^ S) */

#define xorix_rr(XG, XS)                                                    \
        EMITW(0xF3000150 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3000150 | MXM(REG(XG), REG(XG), TmmM))

/* not (G = ~G) */

#define notix_rx(XG)                                                        \
        EMITW(0xF3B005C0 | MXM(REG(XG), 0x00,    REG(XG)))

/**************   packed single precision floating point (NEON)   *************/

/* neg (G = -G) */

#define negis_rx(XG)                                                        \
        EMITW(0xF3B907C0 | MXM(REG(XG), 0x00,    REG(XG)))

/* add (G = G + S) */

#define addis_rr(XG, XS)                                                    \
        EMITW(0xF2000D40 | MXM(REG(XG), REG(XG), REG(XS)))

#define addis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2000D40 | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subis_rr(XG, XS)                                                    \
        EMITW(0xF2200D40 | MXM(REG(XG), REG(XG), REG(XS)))

#define subis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2200D40 | MXM(REG(XG), REG(XG), TmmM))

/* mul (G = G * S) */

#define mulis_rr(XG, XS)                                                    \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), REG(XS)))

#define mulis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), TmmM))

/* div (G = G / S) */

#if RT_SIMD_COMPAT_DIV == 1

#define divis_rr(XG, XS)                                                    \
        EMITW(0xEE800A00 | MXM(REG(XG)+0, REG(XG)+0, REG(XS)+0))            \
        EMITW(0xEEC00AA0 | MXM(REG(XG)+0, REG(XG)+0, REG(XS)+0))            \
        EMITW(0xEE800A00 | MXM(REG(XG)+1, REG(XG)+1, REG(XS)+1))            \
        EMITW(0xEEC00AA0 | MXM(REG(XG)+1, REG(XG)+1, REG(XS)+1))

#define divis_ld(XG, MS, DS)                                                \
        movix_st(V(XG), Mebp, inf_SCR01(0))                                 \
        movix_ld(V(XG), W(MS), W(DS))                                       \
        divis_rr(W(XG), V(XG))                                              \
        movix_ld(V(XG), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_DIV */

#if (RT_128 < 2)

#define divis_rr(XG, XS)                                                    \
        EMITW(0xF3BB0540 | MXM(TmmM,    0x00,    REG(XS))) /* estimate */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    REG(XS))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    REG(XS))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    REG(XS))) /* 3rd N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(TmmC,    REG(XG), TmmM))                     \
        EMITW(0xF2200D50 | MXM(REG(XG), REG(XS), TmmC))    /* residual */   \
        EMITW(0xF2000D50 | MXM(TmmC,    REG(XG), TmmM))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), TmmC,    TmmC))

#define divis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmD,    TPxx,    0x00))                     \
        EMITW(0xF3BB0540 | MXM(TmmM,    0x00,    TmmD))    /* estimate */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    TmmD))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    TmmD))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    TmmD))    /* 3rd N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(TmmC,    REG(XG), TmmM))                     \
        EMITW(0xF2200D50 | MXM(REG(XG), TmmD,    TmmC))    /* residual */   \
        EMITW(0xF2000D50 | MXM(TmmC,    REG(XG), TmmM))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), TmmC,    TmmC))

#else /* RT_128 >= 2 */ /* NOTE: FMA is available in processors with ASIMDv2 */

#define divis_rr(XG, XS)                                                    \
        EMITW(0xF3BB0540 | MXM(TmmM,    0x00,    REG(XS))) /* estimate */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    REG(XS))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(TmmC,    REG(XG), TmmM))                     \
        EMITW(0xF2200C50 | MXM(REG(XG), REG(XS), TmmC))    /* residual */   \
        EMITW(0xF2000C50 | MXM(TmmC,    REG(XG), TmmM))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), TmmC,    TmmC))

#define divis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmD,    TPxx,    0x00))                     \
        EMITW(0xF3BB0540 | MXM(TmmM,    0x00,    TmmD))    /* estimate */   \
        EMITW(0xF2000F50 | MXM(TmmC,    TmmM,    TmmD))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(TmmC,    REG(XG), TmmM))                     \
        EMITW(0xF2200C50 | MXM(REG(XG), TmmD,    TmmC))    /* residual */   \
        EMITW(0xF2000C50 | MXM(TmmC,    REG(XG), TmmM))    /* correction */ \
        EMITW(0xF2200150 | MXM(REG(XG), TmmC,    TmmC))

#endif /* RT_128 >= 2 */

#endif /* RT_SIMD_COMPAT_DIV */

/* sqr (D = sqrt S) */

#if RT_SIMD_COMPAT_SQR == 1

#define sqris_rr(XD, XS)                                                    \
        EMITW(0xEEB10AC0 | MXM(REG(XD)+0, 0x00, REG(XS)+0))                 \
        EMITW(0xEEF10AE0 | MXM(REG(XD)+0, 0x00, REG(XS)+0))                 \
        EMITW(0xEEB10AC0 | MXM(REG(XD)+1, 0x00, REG(XS)+1))                 \
        EMITW(0xEEF10AE0 | MXM(REG(XD)+1, 0x00, REG(XS)+1))

#define sqris_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        sqris_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SQR */

#define sqris_rr(XD, XS)                                                    \
        EMITW(0xF3BB05C0 | MXM(TmmM,    0x00,    REG(XS))) /* estimate */   \
        EMITW(0xF3000D50 | MXM(TmmC,    TmmM,    TmmM))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(TmmC,    TmmC,    REG(XS))) /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(TmmC,    TmmM,    TmmM))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(TmmC,    TmmC,    REG(XS))) /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(XD), REG(XS), TmmM))

#define sqris_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmD,    TPxx,    0x00))                     \
        EMITW(0xF3BB05C0 | MXM(TmmM,    0x00,    TmmD))    /* estimate */   \
        EMITW(0xF3000D50 | MXM(TmmC,    TmmM,    TmmM))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(TmmC,    TmmC,    TmmD))    /* 1st N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(TmmC,    TmmM,    TmmM))    /* pre-mul */    \
        EMITW(0xF2200F50 | MXM(TmmC,    TmmC,    TmmD))    /* 2nd N-R */    \
        EMITW(0xF3000D50 | MXM(TmmM,    TmmM,    TmmC))    /* post-mul */   \
        EMITW(0xF3000D50 | MXM(REG(XD), TmmD,    TmmM))

#endif /* RT_SIMD_COMPAT_SQR */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITW(0xF3BB0540 | MXM(REG(XD), 0x00,    REG(XS)))

#define rcsis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF2000F50 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITW(0xF3BB05C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rssis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF3000D50 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0xF2200F50 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0xF3000D50 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_128 < 2) /* NOTE: only VFP fpu fallback is available for fp32 FMA */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0xF2000D50 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2000D50 | MXM(REG(XG), REG(XS), TmmM))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0xEEB70AC0 | MXM(TmmC+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+0,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+0,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XT)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XT)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XT)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XT)+1))                \
        EMITW(0xEE200B00 | MXM(TmmC+0,  TmmC+0,  TmmC+1))                   \
        EMITW(0xEE200B00 | MXM(TmmD+0,  TmmD+0,  TmmD+1))                   \
        EMITW(0xEE200B00 | MXM(TmmE+0,  TmmE+0,  TmmE+1))                   \
        EMITW(0xEE200B00 | MXM(TmmF+0,  TmmF+0,  TmmF+1))                   \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEE300B00 | MXM(TmmC+1,  TmmC+1,  TmmC+0))                   \
        EMITW(0xEE300B00 | MXM(TmmD+1,  TmmD+1,  TmmD+0))                   \
        EMITW(0xEE300B00 | MXM(TmmE+1,  TmmE+1,  TmmE+0))                   \
        EMITW(0xEE300B00 | MXM(TmmF+1,  TmmF+1,  TmmF+0))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+0, 0x00,  TmmC+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+0, 0x00,  TmmD+1))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+1, 0x00,  TmmE+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+1, 0x00,  TmmF+1))

#define fmais_ld(XG, XS, MT, DT)                                            \
        EMITW(0xEEB70AC0 | MXM(TmmC+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+0,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+0,  0x00,    REG(XS)+1))                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XS), W(MT), W(DT))                                       \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XS)+1))                \
        EMITW(0xEE200B00 | MXM(TmmC+0,  TmmC+0,  TmmC+1))                   \
        EMITW(0xEE200B00 | MXM(TmmD+0,  TmmD+0,  TmmD+1))                   \
        EMITW(0xEE200B00 | MXM(TmmE+0,  TmmE+0,  TmmE+1))                   \
        EMITW(0xEE200B00 | MXM(TmmF+0,  TmmF+0,  TmmF+1))                   \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEE300B00 | MXM(TmmC+1,  TmmC+1,  TmmC+0))                   \
        EMITW(0xEE300B00 | MXM(TmmD+1,  TmmD+1,  TmmD+0))                   \
        EMITW(0xEE300B00 | MXM(TmmE+1,  TmmE+1,  TmmE+0))                   \
        EMITW(0xEE300B00 | MXM(TmmF+1,  TmmF+1,  TmmF+0))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+0, 0x00,  TmmC+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+0, 0x00,  TmmD+1))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+1, 0x00,  TmmE+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+1, 0x00,  TmmF+1))                   \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0xF2200D50 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2200D50 | MXM(REG(XG), REG(XS), TmmM))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0xEEB70AC0 | MXM(TmmC+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+0,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+0,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XT)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XT)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XT)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XT)+1))                \
        EMITW(0xEE200B00 | MXM(TmmC+0,  TmmC+0,  TmmC+1))                   \
        EMITW(0xEE200B00 | MXM(TmmD+0,  TmmD+0,  TmmD+1))                   \
        EMITW(0xEE200B00 | MXM(TmmE+0,  TmmE+0,  TmmE+1))                   \
        EMITW(0xEE200B00 | MXM(TmmF+0,  TmmF+0,  TmmF+1))                   \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEE300B40 | MXM(TmmC+1,  TmmC+1,  TmmC+0))                   \
        EMITW(0xEE300B40 | MXM(TmmD+1,  TmmD+1,  TmmD+0))                   \
        EMITW(0xEE300B40 | MXM(TmmE+1,  TmmE+1,  TmmE+0))                   \
        EMITW(0xEE300B40 | MXM(TmmF+1,  TmmF+1,  TmmF+0))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+0, 0x00,  TmmC+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+0, 0x00,  TmmD+1))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+1, 0x00,  TmmE+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+1, 0x00,  TmmF+1))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        EMITW(0xEEB70AC0 | MXM(TmmC+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+0,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+0,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+0,  0x00,    REG(XS)+1))                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XS), W(MT), W(DT))                                       \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XS)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XS)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XS)+1))                \
        EMITW(0xEE200B00 | MXM(TmmC+0,  TmmC+0,  TmmC+1))                   \
        EMITW(0xEE200B00 | MXM(TmmD+0,  TmmD+0,  TmmD+1))                   \
        EMITW(0xEE200B00 | MXM(TmmE+0,  TmmE+0,  TmmE+1))                   \
        EMITW(0xEE200B00 | MXM(TmmF+0,  TmmF+0,  TmmF+1))                   \
        EMITW(0xEEB70AC0 | MXM(TmmC+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AE0 | MXM(TmmD+1,  0x00,    REG(XG)+0))                \
        EMITW(0xEEB70AC0 | MXM(TmmE+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEEB70AE0 | MXM(TmmF+1,  0x00,    REG(XG)+1))                \
        EMITW(0xEE300B40 | MXM(TmmC+1,  TmmC+1,  TmmC+0))                   \
        EMITW(0xEE300B40 | MXM(TmmD+1,  TmmD+1,  TmmD+0))                   \
        EMITW(0xEE300B40 | MXM(TmmE+1,  TmmE+1,  TmmE+0))                   \
        EMITW(0xEE300B40 | MXM(TmmF+1,  TmmF+1,  TmmF+0))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+0, 0x00,  TmmC+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+0, 0x00,  TmmD+1))                   \
        EMITW(0xEEB70BC0 | MXM(REG(XG)+1, 0x00,  TmmE+1))                   \
        EMITW(0xEEF70BC0 | MXM(REG(XG)+1, 0x00,  TmmF+1))                   \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_128 >= 2 */ /* NOTE: FMA is available in processors with ASIMDv2 */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0xF2000C50 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2000C50 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0xF2200C50 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2200C50 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_128 >= 2 */

/* min (G = G < S ? G : S) */

#define minis_rr(XG, XS)                                                    \
        EMITW(0xF2200F40 | MXM(REG(XG), REG(XG), REG(XS)))

#define minis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2200F40 | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxis_rr(XG, XS)                                                    \
        EMITW(0xF2000F40 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2000F40 | MXM(REG(XG), REG(XG), TmmM))

/* cmp (G = G ? S) */

#define ceqis_rr(XG, XS)                                                    \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), TmmM))

#define cneis_rr(XG, XS)                                                    \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF3B005C0 | MXM(REG(XG), 0x00,    REG(XG)))

#define cneis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2000E40 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0xF3B005C0 | MXM(REG(XG), 0x00,    REG(XG)))

#define cltis_rr(XG, XS)                                                    \
        EMITW(0xF3200E40 | MXM(REG(XG), REG(XS), REG(XG)))

#define cltis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MXM(REG(XG), TmmM,    REG(XG)))

#define cleis_rr(XG, XS)                                                    \
        EMITW(0xF3000E40 | MXM(REG(XG), REG(XS), REG(XG)))

#define cleis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MXM(REG(XG), TmmM,    REG(XG)))

#define cgtis_rr(XG, XS)                                                    \
        EMITW(0xF3200E40 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3200E40 | MXM(REG(XG), REG(XG), TmmM))

#define cgeis_rr(XG, XS)                                                    \
        EMITW(0xF3000E40 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgeis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3000E40 | MXM(REG(XG), REG(XG), TmmM))

/**************************   packed integer (NEON)   *************************/

#if (RT_128 < 4) /* ASIMDv4 is used here for ARMv8:AArch32 processors */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        cvzis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        cvzis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    TmmM))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        cvpis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        cvmis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        cvnis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        cvnis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        cvtis_rr(W(XD), W(XS))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        cvtis_ld(W(XD), W(MS), W(DS))

#else /* RT_128 >= 4 */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF3BA05C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BA05C0 | MXM(REG(XD), 0x00,    TmmM))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BB0740 | MXM(REG(XD), 0x00,    TmmM))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF3BA07C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BA07C0 | MXM(REG(XD), 0x00,    TmmM))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF3BB0240 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BB0240 | MXM(REG(XD), 0x00,    TmmM))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF3BA06C0 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BA06C0 | MXM(REG(XD), 0x00,    TmmM))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF3BB0340 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BB0340 | MXM(REG(XD), 0x00,    TmmM))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF3BA0440 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BA0440 | MXM(REG(XD), 0x00,    TmmM))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF3BB0140 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BB0140 | MXM(REG(XD), 0x00,    TmmM))

#endif /* RT_128 >= 4 */

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF3BB0640 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3BB0640 | MXM(REG(XD), 0x00,    TmmM))

/* add (G = G + S) */

#define addix_rr(XG, XS)                                                    \
        EMITW(0xF2200840 | MXM(REG(XG), REG(XG), REG(XS)))

#define addix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF2200840 | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subix_rr(XG, XS)                                                    \
        EMITW(0xF3200840 | MXM(REG(XG), REG(XG), REG(XS)))

#define subix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3200840 | MXM(REG(XG), REG(XG), TmmM))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlix_ri(XG, IS)                                                    \
        EMITW(0xF2A00550 | MXM(REG(XG), 0x00,    REG(XG)) |                 \
                                                 (0x1F & VAL(IS)) << 16)

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4A00CBF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3200440 | MXM(REG(XG), TmmM,    REG(XG)))

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0xF3200440 | MXM(REG(XG), REG(XS), REG(XG)))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3200440 | MXM(REG(XG), TmmM,    REG(XG)))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrix_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0xF2A00050 | MXM(REG(XG), 0x00,    REG(XG)) |                 \
        (+(VAL(IS) == 0) & 0x00000500) | (+(VAL(IS) != 0) & 0x01000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4A00CBF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(TmmM,    0x00,    TmmM))                     \
        EMITW(0xF3200440 | MXM(REG(XG), TmmM,    REG(XG)))

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0xF3B903C0 | MXM(TmmM,    0x00,    REG(XS)))                  \
        EMITW(0xF3200440 | MXM(REG(XG), TmmM,    REG(XG)))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(TmmM,    0x00,    TmmM))                     \
        EMITW(0xF3200440 | MXM(REG(XG), TmmM,    REG(XG)))


#define shrin_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0xF2A00050 | MXM(REG(XG), 0x00,    REG(XG)) |                 \
        (+(VAL(IS) == 0) & 0x00000500) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4A00CBF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(TmmM,    0x00,    TmmM))                     \
        EMITW(0xF2200440 | MXM(REG(XG), TmmM,    REG(XG)))

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0xF3B903C0 | MXM(TmmM,    0x00,    REG(XS)))                  \
        EMITW(0xF2200440 | MXM(REG(XG), TmmM,    REG(XG)))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0xF3B903C0 | MXM(TmmM,    0x00,    TmmM))                     \
        EMITW(0xF2200440 | MXM(REG(XG), TmmM,    REG(XG)))

/**************************   helper macros (NEON)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x01    /*  all satisfy the condition */

#define movmn_rr(RD, XS) /* not portable, do not use outside */             \
        EMITW(0xF3B60200 | MXM(TmmM+0,  0x00,    REG(XS)))                  \
        EMITW(0xF3B20200 | MXM(TmmM+0,  0x00,    TmmM))                     \
        EMITW(0xEE100B10 | MXM(REG(RD), TmmM+0,  0x00))

#define CHECK_MASK(lb, mask, XS) /* destroys Reax, jump lb if mask == S */  \
        movmn_rr(Reax, W(XS))                                               \
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

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4) /* ASIMDv4 is used here for ARMv8:AArch32 processors */

#define rndis_rr(XD, XS)                                                    \
        cvtis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rndis_ld(XD, MS, DS)                                                \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#else /* RT_128 >= 4 */

#define rndis_rr(XD, XS)     /* fallback to VFP for float-to-integer rnd */ \
        EMITW(0xEEB60A40 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* due to */   \
        EMITW(0xEEF60A60 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* lack of */  \
        EMITW(0xEEB60A40 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* rounding */ \
        EMITW(0xEEF60A60 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* modes */

#define rndis_ld(XD, MS, DS) /* fallback to VFP for float-to-integer rnd */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))                     \
        EMITW(0xEEB60A40 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* due to */   \
        EMITW(0xEEF60A60 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* lack of */  \
        EMITW(0xEEB60A40 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* rounding */ \
        EMITW(0xEEF60A60 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* modes */

#endif /* RT_128 >= 4 */

#define cvtis_rr(XD, XS)     /* fallback to VFP for float-to-integer cvt */ \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* modes */

#define cvtis_ld(XD, MS, DS) /* fallback to VFP for float-to-integer cvt */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))                     \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* due to */   \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* lack of */  \
        EMITW(0xEEBD0A40 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* rounding */ \
        EMITW(0xEEFD0A60 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* modes */

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtin_rr(XD, XS)     /* fallback to VFP for integer-to-float cvt */ \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* due to */   \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+0, 0x00,  REG(XS)+0)) /* lack of */  \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* rounding */ \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+1, 0x00,  REG(XS)+1)) /* modes */

#define cvtin_ld(XD, MS, DS) /* fallback to VFP for integer-to-float cvt */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0xE0800000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0xF4200AAF | MXM(REG(XD), TPxx,    0x00))                     \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* due to */   \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+0, 0x00,  REG(XD)+0)) /* lack of */  \
        EMITW(0xEEB80AC0 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* rounding */ \
        EMITW(0xEEF80AE0 | MXM(REG(XD)+1, 0x00,  REG(XD)+1)) /* modes */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4) /* ASIMDv4 is used here for ARMv8:AArch32 processors */

#define rnris_rr(XD, XS, mode)                                              \
        cvris_rr(W(XD), W(XS), mode)                                        \
        cvnin_rr(W(XD), W(XD))

#define cvris_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_128 >= 4 */

#define rnris_rr(XD, XS, mode)                                              \
        cvris_rr(W(XD), W(XS), mode)                                        \
        cvnin_rr(W(XD), W(XD))

#define cvris_rr(XD, XS, mode)                                              \
        EMITW(0xF3BB0040 | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        ((RT_SIMD_MODE_##mode&3)+1 + 3*(((RT_SIMD_MODE_##mode&3)+1) >> 2)) << 8)

#endif /* RT_128 >= 4 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4000AAF | MXM(TmmM,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4000AAF | MXM(TmmC,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4000AAF | MXM(TmmD,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4000AAF | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4000AAF | MXM(TmmF,    Teax,    0x00))

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movix_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4200AAF | MXM(TmmM,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4200AAF | MXM(TmmC,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4200AAF | MXM(TmmD,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4200AAF | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xF4200AAF | MXM(TmmF,    Teax,    0x00))

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_ARM_128V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

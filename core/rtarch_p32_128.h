/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_128_H
#define RT_RTARCH_P32_128_H

#include "rtarch_p64.h"

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       16
#define RT_SIMD_WIDTH32     4
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH64     2
#define RT_SIMD_SET64(s, v) s[0]=s[1]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la
#undef  movox_ld
#undef  movqx_ld
#define movqx_ld(XD, MS, DS)
#undef  EMITS
#define EMITS(w) EMITW(w)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_128.h: Implementation of Power fp32 VMX/VSX instructions.
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

#define TmmX    0x15  /* v21, +1.0 64-bit */
#define TmmY    0x16  /* v22, -0.5 64-bit */
#define TmmR    0x17  /* v23, Rounding Mode */
#define TmmS    0x18  /* v24, sign-mask 32-bit */
#define TmmQ    0x19  /* v25, full-mask all 1s */
#define TmmA    0x1A  /* v26, +1.0 32-bit */
#define TmmB    0x1B  /* v27, -0.5 32-bit */
#define TmmC    0x1C  /* v28 */
#define TmmD    0x1D  /* v29 */
#define TmmE    0x1E  /* v30 */
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

#if (RT_128 < 2)

/******************************************************************************/
/**********************************   VMX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), REG(XS)))

#define movox_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movox_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C0001CE | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, DS is SIMD-aligned */     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(REG(RD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

/* and (G = G & S) */

#define andox_rr(XG, XS)                                                    \
        EMITW(0x10000404 | MXM(REG(XG), REG(XG), REG(XS)))

#define andox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000404 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        EMITW(0x10000444 | MXM(REG(XG), REG(XS), REG(XG)))

#define annox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000444 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

/* orr (G = G | S) */

#define orrox_rr(XG, XS)                                                    \
        EMITW(0x10000484 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000484 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* orn (G = ~G | S) */

#if (RT_128 < 4)

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

#else /* RT_128 >= 4 */

#define ornox_rr(XG, XS)                                                    \
        EMITW(0x10000544 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000544 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#endif /* RT_128 >= 4 */

/* xor (G = G ^ S) */

#define xorox_rr(XG, XS)                                                    \
        EMITW(0x100004C4 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100004C4 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        EMITW(0x10000504 | MXM(REG(XG), REG(XG), REG(XG)))

/**************   packed single precision floating point (SIMD)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        EMITW(0x100004C4 | MXM(REG(XG), REG(XG), TmmS))

/* add (G = G + S) */

#define addos_rr(XG, XS)                                                    \
        EMITW(0x1000000A | MXM(REG(XG), REG(XG), REG(XS)))

#define addos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000000A | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subos_rr(XG, XS)                                                    \
        EMITW(0x1000004A | MXM(REG(XG), REG(XG), REG(XS)))

#define subos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000004A | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* mul (G = G * S) */

#define mulos_rr(XG, XS)                                                    \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), TmmS) | REG(XS) << 6)

#define mulos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), TmmS) | Tmm1 << 6)

/* internal definitions for IEEE-compatible div & sqr */

#define movfx_ld(fd, MS, DS) /* not portable, do not use outside */         \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(fd,      MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movfx_st(fs, MD, DD) /* not portable, do not use outside */         \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xD0000000 | MDM(fs,      MOD(MD), VAL(DD), B1(DD), P1(DD)))

#define divfs_rr(fg, fs) /* not portable, do not use outside */             \
        EMITW(0xEC000024 | MTM(fg,      fg,      fs))

#define sqrfs_rr(fd, fs) /* not portable, do not use outside */             \
        EMITW(0xEC00002C | MTM(fd,      0x00,    fs))

/* div (G = G / S) */

#if RT_SIMD_COMPAT_DIV == 1

#define divos_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x04))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x08))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x0C))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define divos_ld(XG, MS, DS)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x04))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x08))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movfx_ld(Tff2,  Mebp, inf_SCR02(0x0C))                              \
        divfs_rr(Tff1, Tff2)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_DIV */

#define divos_rr(XG, XS)                                                    \
        EMITW(0x1000010A | MXM(TmmC,    0x00,    REG(XS)))                  \
        EMITW(0x1000002F | MXM(TmmD,    TmmC,    TmmA) | REG(XS) << 6)      \
        EMITW(0x1000002E | MXM(TmmC,    TmmC,    TmmC) | TmmD << 6)         \
        EMITW(0x1000002E | MXM(TmmD,    REG(XG), TmmS) | TmmC << 6)         \
        EMITW(0x1000002F | MXM(REG(XG), TmmD, REG(XG)) | REG(XS) << 6)      \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), TmmD) | TmmC << 6)

#define divos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000010A | MXM(TmmC,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x1000002F | MXM(TmmD,    TmmC,    TmmA) | Tmm1 << 6)         \
        EMITW(0x1000002E | MXM(TmmC,    TmmC,    TmmC) | TmmD << 6)         \
        EMITW(0x1000002E | MXM(TmmD,    REG(XG), TmmS) | TmmC << 6)         \
        EMITW(0x1000002F | MXM(REG(XG), TmmD, REG(XG)) | Tmm1 << 6)         \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), TmmD) | TmmC << 6)

#endif /* RT_SIMD_COMPAT_DIV */

/* sqr (D = sqrt S) */

#if RT_SIMD_COMPAT_SQR == 1

#define sqros_rr(XD, XS)                                                    \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define sqros_ld(XD, MS, DS)                                                \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        movox_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movfx_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        sqrfs_rr(Tff1, Tff1)                                                \
        movfx_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_SQR */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(TmmC,    0x00,    REG(XS)))                  \
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmS) | TmmC << 6)         \
        EMITW(0x1000002E | MXM(TmmE,    TmmC,    TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmC,    TmmD,    TmmC) | TmmE << 6)         \
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmS) | TmmC << 6)         \
        EMITW(0x1000002E | MXM(TmmE,    TmmC,    TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmC,    TmmD,    TmmC) | TmmE << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmC,    TmmS) | REG(XS) << 6)

#define sqros_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000014A | MXM(TmmC,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmS) | TmmC << 6)         \
        EMITW(0x1000002E | MXM(TmmE,    TmmC,    TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | Tmm1 << 6)         \
        EMITW(0x1000002F | MXM(TmmC,    TmmD,    TmmC) | TmmE << 6)         \
        EMITW(0x1000002E | MXM(TmmD,    TmmC,    TmmS) | TmmC << 6)         \
        EMITW(0x1000002E | MXM(TmmE,    TmmC,    TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | Tmm1 << 6)         \
        EMITW(0x1000002F | MXM(TmmC,    TmmD,    TmmC) | TmmE << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmC,    TmmS) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_SQR */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        EMITW(0x1000010A | MXM(REG(XD), 0x00,    REG(XS)))

#define rcsos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0x1000002F | MXM(REG(XS), REG(XG), TmmA) | REG(XS) << 6)      \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), REG(XG)) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(REG(XD), 0x00,    REG(XS)))

#define rssos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0x1000002E | MXM(TmmD,    REG(XG), TmmS) | REG(XG) << 6)      \
        EMITW(0x1000002E | MXM(TmmC,    REG(XG), TmmS) | TmmB << 6)         \
        EMITW(0x1000002F | MXM(TmmD,    TmmD,    TmmA) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(REG(XG), TmmD,    REG(XG)) | TmmC << 6)

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | Tmm1 << 6)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | Tmm1 << 6)

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minos_rr(XG, XS)                                                    \
        EMITW(0x1000044A | MXM(REG(XG), REG(XG), REG(XS)))

#define minos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000044A | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* max (G = G > S ? G : S) */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0x1000040A | MXM(REG(XG), REG(XG), REG(XS)))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000040A | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* cmp (G = G ? S) */

#define ceqos_rr(XG, XS)                                                    \
        EMITW(0x100000C6 | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100000C6 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#define cneos_rr(XG, XS)                                                    \
        EMITW(0x100000C6 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XG), REG(XG), REG(XG)))

#define cneos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100000C6 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */\
        EMITW(0x10000504 | MXM(REG(XG), REG(XG), REG(XG)))

#define cltos_rr(XG, XS)                                                    \
        EMITW(0x100002C6 | MXM(REG(XG), REG(XS), REG(XG)))

#define cltos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100002C6 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#define cleos_rr(XG, XS)                                                    \
        EMITW(0x100001C6 | MXM(REG(XG), REG(XS), REG(XG)))

#define cleos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100001C6 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#define cgtos_rr(XG, XS)                                                    \
        EMITW(0x100002C6 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100002C6 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#define cgeos_rr(XG, XS)                                                    \
        EMITW(0x100001C6 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgeos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100001C6 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x100003CA | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003CA | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    REG(XS)))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        rnnos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        rnnos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x1000034A | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000034A | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#else /* RT_128 >= 2 */

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov (D = S) */

#define movox_rr(XG, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XS), REG(XS)))

#define movox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(REG(XG), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movox_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */


#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, DS is SIMD-aligned */     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(REG(RD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

/* and (G = G & S) */

#define andox_rr(XG, XS)                                                    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), REG(XS)))

#define andox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        EMITW(0xF0000457 | MXM(REG(XG), REG(XS), REG(XG)))

#define annox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

/* orr (G = G | S) */

#define orrox_rr(XG, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* orn (G = ~G | S) */

#if (RT_128 < 4)

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

#else /* RT_128 >= 4 */

#define ornox_rr(XG, XS)                                                    \
        EMITW(0xF0000557 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#endif /* RT_128 >= 4 */

/* xor (G = G ^ S) */

#define xorox_rr(XG, XS)                                                    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

/**************   packed single precision floating point (SIMD)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        EMITW(0xF00006E7 | MXM(REG(XG), 0x00,    REG(XG)))

/* add (G = G + S) */

#define addos_rr(XG, XS)                                                    \
        EMITW(0xF0000207 | MXM(REG(XG), REG(XG), REG(XS)))

#define addos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000207 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subos_rr(XG, XS)                                                    \
        EMITW(0xF0000247 | MXM(REG(XG), REG(XG), REG(XS)))

#define subos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000247 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* mul (G = G * S) */

#define mulos_rr(XG, XS)                                                    \
        EMITW(0xF0000287 | MXM(REG(XG), REG(XG), REG(XS)))

#define mulos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000287 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* div (G = G / S) */

#define divos_rr(XG, XS)                                                    \
        EMITW(0xF00002C7 | MXM(REG(XG), REG(XG), REG(XS)))

#define divos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002C7 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    REG(XS)))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        EMITW(0xF000026B | MXM(REG(XD), 0x00,    REG(XS)))

#define rcsos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0xF00006CF | MXM(REG(XS), REG(XG), TmmA))                     \
        EMITW(0xF000020F | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        EMITW(0xF000022B | MXM(REG(XD), 0x00,    REG(XS)))

#define rssos_rr(XG, XS) /* destroys MS */                                  \
        EMITW(0xF0000287 | MXM(TmmD,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000287 | MXM(TmmC,    REG(XG), TmmB))                     \
        EMITW(0xF00006CF | MXM(TmmD,    REG(XS), TmmA))                     \
        EMITW(0xF000068F | MXM(REG(XG), TmmD,    TmmC))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), REG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), Tmm1))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), Tmm1))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minos_rr(XG, XS)                                                    \
        EMITW(0xF0000647 | MXM(REG(XG), REG(XG), REG(XS)))

#define minos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000647 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* max (G = G > S ? G : S) */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0xF0000607 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000607 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* cmp (G = G ? S) */

#define ceqos_rr(XG, XS)                                                    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#define cneos_rr(XG, XS)                                                    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cneos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000021F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cltos_rr(XG, XS)                                                    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XS), REG(XG)))

#define cltos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#define cleos_rr(XG, XS)                                                    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XS), REG(XG)))

#define cleos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(REG(XG), Tmm1,    REG(XG)))/* ^ == -1 if true */

#define cgtos_rr(XG, XS)                                                    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000025F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#define cgeos_rr(XG, XS)                                                    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgeos_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000029F | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        rnnos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        rnnos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

#endif /* RT_128 >= 2 */

/**************************   packed integer (SIMD)   *************************/

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), REG(XS)))

#define addox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), REG(XS)))

#define subox_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), Tmm1))/* ^ == -1 if true */

#if RT_ENDIAN == 0

/* shl (G = G << S) */

#define shlox_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), Tmm1))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(Tmm1,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), Tmm1))

/* shr (G = G >> S) */

#define shrox_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), Tmm1))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(Tmm1,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), Tmm1))

#define shron_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), Tmm1))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(Tmm1,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), Tmm1))

#else /* RT_ENDIAN == 0 */

/* shl (G = G << S) */

#define shlox_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), Tmm1))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(Tmm1,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), Tmm1))

/* shr (G = G >> S) */

#define shrox_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), Tmm1))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(Tmm1,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), Tmm1))

#define shron_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(Tmm1,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), Tmm1))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(Tmm1,    0x00,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), Tmm1))

#endif /* RT_ENDIAN == 0 */

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       MN      /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       MF      /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask
#define SMN(rg, lb) ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END
#define SMF(rg, lb) ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define CHECK_MASK(lb, mask, XS) /* destroys Reax, jump lb if mask == S */  \
        EMITW(0x10000486 | MXM(REG(XS), REG(XS), TmmQ))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

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

#define fpscr_ld(RS) /* not portable, do not use outside */                 \
        EMITW(0xFE00058E | MRM(0x00,    REG(RS), 0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xFC00048E | MRM(REG(RD), 0x00,    0x00))

#if (RT_128 < 2)

#define F0(mode)    F1(mode)
#define F1(mode)    F##mode
#define F0x00                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmS))
#define F0x03                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmB))
#define F0x02                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmB))
#define F0x01                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmS)) /*!*/
#define F0x04                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmS))
#define F0x07                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmB))
#define F0x06                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmB))
#define F0x05                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmS)) /*!*/

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        F0(RT_SIMD_MODE_##mode)

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        F0(RT_SIMD_MODE_ROUNDN)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    REG(XS)))                  \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    Tmm1))/* ^ == -1 if true */\
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define cvtos_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvtos_ld(XD, MS, DS)                                                \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        cvnon_rr(W(XD), W(XS))                                            /*!*/

#define cvton_ld(XD, MS, DS)                                                \
        cvnon_ld(W(XD), W(MS), W(DS))                                     /*!*/

#else /* RT_128 >= 2 */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xFF80010C | RT_SIMD_MODE_##mode << 12)

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xFF80010C)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#define cvtos_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvtos_ld(XD, MS, DS)                                                \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvton_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(Tmm1,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    Tmm1))/* ^ == -1 if true */

#endif /* RT_128 >= 2 */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define cvros_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)                                        \
        cvzos_rr(W(XD), W(XD))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

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
        EMITW(0x7C0001CE | MXM(TmmX,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmY,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmR,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmS,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmA,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmB,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmC,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmD,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0001CE | MXM(Tmm1,    0x00,    Teax))

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
        EMITW(0x7C0000CE | MXM(TmmX,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmY,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmR,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmS,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmA,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmB,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmC,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmD,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C0000CE | MXM(Tmm1,    0x00,    Teax))

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_128V1_H
#define RT_RTARCH_P32_128V1_H

#include "rtarch_p64.h"

#define RT_SIMD_REGS_128        16
#define RT_SIMD_ALIGN_128       16
#define RT_SIMD_WIDTH64_128     2
#define RT_SIMD_SET64_128(s, v) s[0]=s[1]=v
#define RT_SIMD_WIDTH32_128     4
#define RT_SIMD_SET32_128(s, v) s[0]=s[1]=s[2]=s[3]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_128v1.h: Implementation of Power fp32 VMX instructions.
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
 * cmdi*_** - applies [cmd] to 32-bit SIMD element args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit SIMD element args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size SIMD element args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit SIMD element args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit SIMD element args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size SIMD element args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD element args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size SIMD element args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit SIMD element args, packed-var-len
 *
 * cmd*x_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * The cmdp*_** (rtbase.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtbase.h as well.
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing of 256/512-bit.
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

#if defined (RT_128) && (RT_128 == 1)

#undef  sregs_sa
#undef  sregs_la

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

#define TmmR    0x18  /* v24, VMX only, Rounding Mode */
#define TmmS    0x19  /* v25, VMX only, sign-mask 32-bit */
#define TmmT    0x1E  /* v30, VMX only */
#define TmmU    0x1A  /* v26, VMX only, +1.0 32-bit */
#define TmmV    0x1B  /* v27, VMX only, -0.5 32-bit */
#define TmmW    0x1C  /* v28, VMX only */
#define TmmZ    0x1D  /* v29, VMX only */

#define TmmE    0x0E  /* v14, internal name for XmmE (in sregs) */
#define TmmF    0x10  /* v16, internal name for XmmF (in sregs) */
#define TmmQ    0x0F  /* v15, internal name for all-ones */
#define TmmM    0x1F  /* v31, temp-reg name for mem-args */

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
#if     RT_SIMD_COMPAT_XMM < 2
#define XmmE    TmmE, 0x00, EMPTY       /* v14, may be reserved in some cases */
#if     RT_SIMD_COMPAT_XMM < 1
#define XmmF    TmmF, 0x00, EMPTY       /* v16, may be reserved in some cases */
#endif/*RT_SIMD_COMPAT_XMM < 1*/
#endif/*RT_SIMD_COMPAT_XMM < 2*/

/* The last two SIMD registers can be reserved by the assembler when building
 * RISC targets with SIMD wider than natively supported 128-bit, in which case
 * they will be occupied by temporary data. Two hidden registers may also come
 * in handy when implementing elaborate register-spill techniques in the future
 * for current targets with less native registers than architecturally exposed.
 *
 * It should be possible to reserve only 1 SIMD register (XmmF) to achieve the
 * goals above (totalling 15 regs) at the cost of extra loads in certain ops. */

/******************************************************************************/
/**********************************   VMX   ***********************************/
/******************************************************************************/

/* adr (D = adr S) */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, MS/DS is SIMD-aligned */  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(REG(RD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), REG(XS)))

#define movix_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movix_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C0001CE | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvix_rr(XG, XS)                                                    \
        EMITW(0x1000002A | MXM(REG(XG), REG(XG), REG(XS)))

#define mmvix_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000002A | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define mmvix_st(XS, MG, DG)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0x1000002A | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C0001CE | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* and (G = G & S), (D = S & T) if (D != S) */

#define andix_rr(XG, XS)                                                    \
        andix3rr(W(XG), W(XG), W(XS))

#define andix_ld(XG, MS, DS)                                                \
        andix3ld(W(XG), W(XG), W(MS), W(DS))

#define andix3rr(XD, XS, XT)                                                \
        EMITW(0x10000404 | MXM(REG(XD), REG(XS), REG(XT)))

#define andix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x10000404 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* ann (G = ~G & S), (D = ~S & T) if (D != S) */

#define annix_rr(XG, XS)                                                    \
        annix3rr(W(XG), W(XG), W(XS))

#define annix_ld(XG, MS, DS)                                                \
        annix3ld(W(XG), W(XG), W(MS), W(DS))

#define annix3rr(XD, XS, XT)                                                \
        EMITW(0x10000444 | MXM(REG(XD), REG(XT), REG(XS)))

#define annix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x10000444 | MXM(REG(XD), TmmM,    REG(XS)))/* ^ == -1 if true */

/* orr (G = G | S), (D = S | T) if (D != S) */

#define orrix_rr(XG, XS)                                                    \
        orrix3rr(W(XG), W(XG), W(XS))

#define orrix_ld(XG, MS, DS)                                                \
        orrix3ld(W(XG), W(XG), W(MS), W(DS))

#define orrix3rr(XD, XS, XT)                                                \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* orn (G = ~G | S), (D = ~S | T) if (D != S) */

#define ornix_rr(XG, XS)                                                    \
        notix_rx(W(XG))                                                     \
        orrix_rr(W(XG), W(XS))

#define ornix_ld(XG, MS, DS)                                                \
        notix_rx(W(XG))                                                     \
        orrix_ld(W(XG), W(MS), W(DS))

#define ornix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        ornix_rr(W(XD), W(XT))

#define ornix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        ornix_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (D != S) */

#define xorix_rr(XG, XS)                                                    \
        xorix3rr(W(XG), W(XG), W(XS))

#define xorix_ld(XG, MS, DS)                                                \
        xorix3ld(W(XG), W(XG), W(MS), W(DS))

#define xorix3rr(XD, XS, XT)                                                \
        EMITW(0x100004C4 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x100004C4 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* not (G = ~G) */

#define notix_rx(XG)                                                        \
        EMITW(0x10000504 | MXM(REG(XG), REG(XG), REG(XG)))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G) */

#define negis_rx(XG)                                                        \
        EMITW(0x100004C4 | MXM(REG(XG), REG(XG), TmmS))

/* add (G = G + S), (D = S + T) if (D != S) */

#define addis_rr(XG, XS)                                                    \
        addis3rr(W(XG), W(XG), W(XS))

#define addis_ld(XG, MS, DS)                                                \
        addis3ld(W(XG), W(XG), W(MS), W(DS))

#define addis3rr(XD, XS, XT)                                                \
        EMITW(0x1000000A | MXM(REG(XD), REG(XS), REG(XT)))

#define addis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000000A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subis_rr(XG, XS)                                                    \
        subis3rr(W(XG), W(XG), W(XS))

#define subis_ld(XG, MS, DS)                                                \
        subis3ld(W(XG), W(XG), W(MS), W(DS))

#define subis3rr(XD, XS, XT)                                                \
        EMITW(0x1000004A | MXM(REG(XD), REG(XS), REG(XT)))

#define subis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000004A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* mul (G = G * S), (D = S * T) if (D != S) */

#define mulis_rr(XG, XS)                                                    \
        mulis3rr(W(XG), W(XG), W(XS))

#define mulis_ld(XG, MS, DS)                                                \
        mulis3ld(W(XG), W(XG), W(MS), W(DS))

#define mulis3rr(XD, XS, XT)                                                \
        EMITW(0x1000002E | MXM(REG(XD), REG(XS), TmmS) | REG(XT) << 6)

#define mulis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000002E | MXM(REG(XD), REG(XS), TmmS) | TmmM << 6)

/* div (G = G / S), (D = S / T) if (D != S) */

#define divis_rr(XG, XS)                                                    \
        divis3rr(W(XG), W(XG), W(XS))

#define divis_ld(XG, MS, DS)                                                \
        divis3ld(W(XG), W(XG), W(MS), W(DS))

/* internal definitions for IEEE-compatible div & sqr */

#define movws_ld(fd, MS, DS) /* not portable, do not use outside */         \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(fd,      MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movws_st(fs, MD, DD) /* not portable, do not use outside */         \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xD0000000 | MDM(fs,      MOD(MD), VAL(DD), B1(DD), P1(DD)))

#define divws_rr(fg, fs) /* not portable, do not use outside */             \
        EMITW(0xEC000024 | MTM(fg,      fg,      fs))

#define sqrws_rr(fd, fs) /* not portable, do not use outside */             \
        EMITW(0xEC00002C | MTM(fd,      0x00,    fs))

#if RT_SIMD_COMPAT_DIV == 1

#define divis3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x04))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x08))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x0C))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define divis3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x04))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x08))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x0C))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_DIV */

#define divis3rr(XD, XS, XT)                                                \
        EMITW(0x1000010A | MXM(TmmW,    0x00,    REG(XT)))                  \
        EMITW(0x1000002F | MXM(TmmZ,    TmmW,    TmmU) | REG(XT) << 6)      \
        EMITW(0x1000002E | MXM(TmmW,    TmmW,    TmmW) | TmmZ << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XS), TmmS) | TmmW << 6)         \
        EMITW(0x1000002F | MXM(REG(XD), TmmZ, REG(XS)) | REG(XT) << 6)      \
        EMITW(0x1000002E | MXM(REG(XD), REG(XD), TmmZ) | TmmW << 6)

#define divis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000010A | MXM(TmmW,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x1000002F | MXM(TmmZ,    TmmW,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002E | MXM(TmmW,    TmmW,    TmmW) | TmmZ << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XS), TmmS) | TmmW << 6)         \
        EMITW(0x1000002F | MXM(REG(XD), TmmZ, REG(XS)) | TmmM << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), REG(XD), TmmZ) | TmmW << 6)

#endif /* RT_SIMD_COMPAT_DIV */

/* sqr (D = sqrt S) */

#if RT_SIMD_COMPAT_SQR == 1

#define sqris_rr(XD, XS)                                                    \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define sqris_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_SQR */

#define sqris_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(TmmW,    0x00,    REG(XS)))                  \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmW,    TmmS) | REG(XS) << 6)

#define sqris_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000014A | MXM(TmmW,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmW,    TmmS) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_SQR */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITW(0x1000010A | MXM(REG(XD), 0x00,    REG(XS)))

#define rcsis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1000002F | MXM(REG(XS), REG(XG), TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), REG(XG)) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(REG(XD), 0x00,    REG(XS)))

#define rssis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XG), TmmS) | REG(XG) << 6)      \
        EMITW(0x1000002E | MXM(TmmW,    REG(XG), TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(REG(XG), TmmZ,    REG(XG)) | TmmW << 6)

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (D != S) */

#define minis_rr(XG, XS)                                                    \
        minis3rr(W(XG), W(XG), W(XS))

#define minis_ld(XG, MS, DS)                                                \
        minis3ld(W(XG), W(XG), W(MS), W(DS))

#define minis3rr(XD, XS, XT)                                                \
        EMITW(0x1000044A | MXM(REG(XD), REG(XS), REG(XT)))

#define minis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000044A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (D != S) */

#define maxis_rr(XG, XS)                                                    \
        maxis3rr(W(XG), W(XG), W(XS))

#define maxis_ld(XG, MS, DS)                                                \
        maxis3ld(W(XG), W(XG), W(MS), W(DS))

#define maxis3rr(XD, XS, XT)                                                \
        EMITW(0x1000040A | MXM(REG(XD), REG(XS), REG(XT)))

#define maxis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x1000040A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* ceq (G = G == S ? 1 : 0), (D = S == T ? 1 : 0) if (D != S) */

#define ceqis_rr(XG, XS)                                                    \
        ceqis3rr(W(XG), W(XG), W(XS))

#define ceqis_ld(XG, MS, DS)                                                \
        ceqis3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqis3rr(XD, XS, XT)                                                \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* cne (G = G != S ? 1 : 0), (D = S != T ? 1 : 0) if (D != S) */

#define cneis_rr(XG, XS)                                                    \
        cneis3rr(W(XG), W(XG), W(XS))

#define cneis_ld(XG, MS, DS)                                                \
        cneis3ld(W(XG), W(XG), W(MS), W(DS))

#define cneis3rr(XD, XS, XT)                                                \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cneis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* clt (G = G < S ? 1 : 0), (D = S < T ? 1 : 0) if (D != S) */

#define cltis_rr(XG, XS)                                                    \
        cltis3rr(W(XG), W(XG), W(XS))

#define cltis_ld(XG, MS, DS)                                                \
        cltis3ld(W(XG), W(XG), W(MS), W(DS))

#define cltis3rr(XD, XS, XT)                                                \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x100002C6 | MXM(REG(XD), TmmM,    REG(XS)))/* ^ == -1 if true */

/* cle (G = G <= S ? 1 : 0), (D = S <= T ? 1 : 0) if (D != S) */

#define cleis_rr(XG, XS)                                                    \
        cleis3rr(W(XG), W(XG), W(XS))

#define cleis_ld(XG, MS, DS)                                                \
        cleis3ld(W(XG), W(XG), W(MS), W(DS))

#define cleis3rr(XD, XS, XT)                                                \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XT), REG(XS)))

#define cleis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x100001C6 | MXM(REG(XD), TmmM,    REG(XS)))/* ^ == -1 if true */

/* cgt (G = G > S ? 1 : 0), (D = S > T ? 1 : 0) if (D != S) */

#define cgtis_rr(XG, XS)                                                    \
        cgtis3rr(W(XG), W(XG), W(XS))

#define cgtis_ld(XG, MS, DS)                                                \
        cgtis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtis3rr(XD, XS, XT)                                                \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* cge (G = G >= S ? 1 : 0), (D = S >= T ? 1 : 0) if (D != S) */

#define cgeis_rr(XG, XS)                                                    \
        cgeis3rr(W(XG), W(XG), W(XS))

#define cgeis_ld(XG, MS, DS)                                                \
        cgeis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeis3rr(XD, XS, XT)                                                \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_128  MN32_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128  MF32_128   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x10000486 | MXM(REG(XS), REG(XS), TmmQ))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##32_128), EMPTY2)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x100003CA | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003CA | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        rnpis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    REG(XS)))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        rnmis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        rnnis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        rnnis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x1000034A | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000034A | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    REG(XS)))                  \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define rndis_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    TmmM))/* ^ == -1 if true */\
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define cvtis_rr(XD, XS)                                                    \
        rndis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvtis_ld(XD, MS, DS)                                                \
        rndis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtin_rr(XD, XS)                                                    \
        cvnin_rr(W(XD), W(XS))                                            /*!*/

#define cvtin_ld(XD, MS, DS)                                                \
        cvnin_ld(W(XD), W(MS), W(DS))                                     /*!*/

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define cvris_rr(XD, XS, mode)                                              \
        rnris_rr(W(XD), W(XS), mode)                                        \
        cvzis_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S) */

#define addix_rr(XG, XS)                                                    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), REG(XS)))

#define addix_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000080 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subix_rr(XG, XS)                                                    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), REG(XS)))

#define subix_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000480 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlix_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))

#if RT_ENDIAN == 0

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), REG(XS)))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000184 | MXM(REG(XG), REG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrix_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))

#if RT_ENDIAN == 0

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), REG(XS)))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000284 | MXM(REG(XG), REG(XG), TmmM))


#define shrin_ri(XG, IS)                                                    \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IS)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))

#if RT_ENDIAN == 0

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1003028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))

#else /* RT_ENDIAN == 1 */

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x1000028C | MXM(TmmM,    0x00,    TmmM))/* ^ == -1 if true */\
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))

#endif /* RT_ENDIAN == 1 */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), REG(XS)))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x10000384 | MXM(REG(XG), REG(XG), TmmM))

/************************   helper macros (FPU mode)   ************************/

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

#define F0(mode)    F1(mode)
#define F1(mode)    F##mode
#define F0x00                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmS))
#define F0x03                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmV))
#define F0x02                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmV))
#define F0x01                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmS)) /*!*/
#define F0x04                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmS))
#define F0x07                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmV))
#define F0x06                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmV))
#define F0x05                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmS)) /*!*/

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        F0(RT_SIMD_MODE_##mode)

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        F0(RT_SIMD_MODE_ROUNDN)

/***************   scalar single-precision floating-point move   **************/

/* mov (D = S) */

#define movrx_rr(XD, XS)                                                    \
        EMITW(0xFC000090 | MXM(REG(XD), 0x00,    REG(XS)))

#define movrx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(REG(XD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movrx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xD0000000 | MDM(REG(XS), MOD(MD), VAL(DD), B1(DD), P1(DD)))

/************   scalar single-precision floating-point arithmetic   ***********/

/* add (G = G + S) */

#define addrs_rr(XG, XS)                                                    \
        EMITW(0xEC00002A | MXM(REG(XG), REG(XG), REG(XS)))

#define addrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEC00002A | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subrs_rr(XG, XS)                                                    \
        EMITW(0xEC000028 | MXM(REG(XG), REG(XG), REG(XS)))

#define subrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEC000028 | MXM(REG(XG), REG(XG), TmmM))

/* mul (G = G * S) */

#define mulrs_rr(XG, XS)                                                    \
        EMITW(0xEC000032 | MXM(REG(XG), REG(XG), 0x00) | REG(XS) << 6)

#define mulrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEC000032 | MXM(REG(XG), REG(XG), 0x00) | TmmM << 6)

/* div (G = G / S) */

#define divrs_rr(XG, XS)                                                    \
        EMITW(0xEC000024 | MXM(REG(XG), REG(XG), REG(XS)))

#define divrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEC000024 | MXM(REG(XG), REG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
        EMITW(0xEC00002C | MXM(REG(XD), 0x00,    REG(XS)))

#define sqrrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC0000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEC00002C | MXM(REG(XD), 0x00,    TmmM))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrx_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
        sqrrs_rr(W(XD), W(XS))                                              \
        movrx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movrx_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMITW(0xEC00003A | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xC0000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0xEC00003A | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMITW(0xEC00003C | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xC0000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0xEC00003C | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   scalar single-precision floating-point compare   *************/

/* min (G = G < S ? G : S) */

#define minrs_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        minis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define minrs_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        minis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S) */

#define maxrs_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        maxis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define maxrs_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        maxis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? 1 : 0), (D = S == T ? 1 : 0) if (D != S) */

#define ceqrs_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        ceqis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define ceqrs_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        ceqis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/* cne (G = G != S ? 1 : 0), (D = S != T ? 1 : 0) if (D != S) */

#define cners_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cneis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define cners_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cneis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/* clt (G = G < S ? 1 : 0), (D = S < T ? 1 : 0) if (D != S) */

#define cltrs_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cltis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define cltrs_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cltis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/* cle (G = G <= S ? 1 : 0), (D = S <= T ? 1 : 0) if (D != S) */

#define clers_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cleis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define clers_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cleis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? 1 : 0), (D = S > T ? 1 : 0) if (D != S) */

#define cgtrs_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cgtis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define cgtrs_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cgtis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/* cge (G = G >= S ? 1 : 0), (D = S >= T ? 1 : 0) if (D != S) */

#define cgers_rr(XG, XS)                                                    \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cgeis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

#define cgers_ld(XG, MS, DS)                                                \
        movrx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), W(MS), W(DS))                                       \
        movrx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        cgeis_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movrx_ld(W(XG), Mebp, inf_SCR01(0))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#define movtx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(REG(XD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movtx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xD8000000 | MDM(REG(XS), MOD(MD), VAL(DD), B1(DD), P1(DD)))

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0001CE | MXM(TmmR,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0001CE | MXM(TmmS,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0001CE | MXM(TmmT,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0001CE | MXM(TmmU,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0001CE | MXM(TmmV,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0001CE | MXM(TmmW,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0001CE | MXM(TmmZ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0001CE | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0001CE | MXM(TmmF,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0001CE | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0001CE | MXM(TmmM,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITP(0x7C000718 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITP(0x7C000718 | MXM(TmmM,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0xD8000000 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0xD8000000 | MXM(TmmF,    Teax,    0x00))

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0000CE | MXM(TmmR,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0000CE | MXM(TmmS,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0000CE | MXM(TmmT,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0000CE | MXM(TmmU,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0000CE | MXM(TmmV,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0000CE | MXM(TmmW,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITM(0x7C0000CE | MXM(TmmZ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0000CE | MXM(TmmE,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0000CE | MXM(TmmF,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0000CE | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C0000CE | MXM(TmmM,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITP(0x7C000618 | MXM(TmmQ,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITP(0x7C000618 | MXM(TmmM,    0x00,    Teax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movtx_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0xC8000000 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0xC8000000 | MXM(TmmF,    Teax,    0x00))

#ifndef RT_RTARCH_P32_256V8_H
#undef  RT_256
#define RT_256  8
#include "rtarch_p32_256v8.h"
#endif /* RT_RTARCH_P32_256V8_H */

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_128V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

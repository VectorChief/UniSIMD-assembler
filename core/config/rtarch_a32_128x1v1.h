/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_128X1V1_H
#define RT_RTARCH_A32_128X1V1_H

#include "rtarch_a64.h"

#define RT_SIMD_REGS_128        32

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a32_128x1v1.h: Implementation of ARMv8:AArch64 fp32 NEON instructions.
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
 * The cmdp*_** (rtconf.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data elements (fp+int).
 * In this model data paths are fixed-width, BASE and SIMD data elements are
 * width-compatible, code path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtconf.h as well.
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing with 512-bit
 * and wider vectors. Use of scalars may leave respective vector registers
 * undefined, as seen from the perspective of any particular vector subset.
 *
 * 256-bit vectors used with wider subsets may not be compatible with regards
 * to memory loads/stores when mixed in the code. It means that data loaded
 * with wider vector and stored within 256-bit subset at the same address may
 * result in changing the initial representation in memory. The same can be
 * said about mixing vector and scalar subsets. Scalars can be completely
 * detached on some architectures. Use elm*x_st to store 1st vector element.
 * 128-bit vectors should be memory-compatible with any wider vector subset.
 *
 * Handling of NaNs in the floating point pipeline may not be consistent
 * across different architectures. Avoid NaNs entering the data flow by using
 * masking or control flow instructions. Apply special care when dealing with
 * floating point compare and min/max input/output. The result of floating point
 * compare instructions can be considered a -QNaN, though it is also interpreted
 * as integer -1 and is often treated as a mask. Most arithmetic instructions
 * should propagate QNaNs unchanged, however this behavior hasn't been verified.
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

#if (defined RT_SIMD_CODE)

#if (RT_128X1 != 0)

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 5 | (reg))

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 5 | (reg))

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  B4(val, tp1, tp2)  B4##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  L2(val, tp1, tp2)  L2##tp2
#define  C2(val, tp1, tp2)  C2##tp2
#define  A2(val, tp1, tp2)  A2##tp2

/* displacement encoding SIMD(TP2), ELEM(TP1) */

#define B20(br) (br)
#define B40(br) (br)
#define P20(dp) (0x01000000 | ((dp) & 0xFFF0) << 6)
#define L20(dp) (0x00000000 | ((dp) & 0xFFF0) << 6)
#define C20(br, dp) EMPTY
#define A20(br, dp) EMPTY

#define B21(br) (br)
#define B41(br) TPxx
#define P21(dp) (0x00206800 | TDxx << 16)
#define L21(dp) (0x00000000 | ((dp) & 0x0010) << 6)
#define C21(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0xFFFC & (dp)) << 5)
#define A21(br, dp) C21(br, dp)                                             \
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx) | ADR)

#define B22(br) (br)
#define B42(br) TPxx
#define P22(dp) (0x00206800 | TDxx << 16)
#define L22(dp) (0x00000000 | ((dp) & 0x0010) << 6)
#define C22(br, dp) EMITW(0x52800000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0xFFF0 & (dp)) << 5)                          \
                    EMITW(0x72A00000 | MRM(TDxx,    0x00,    0x00) |        \
                             (0x7FFF & (dp) >> 16) << 5)
#define A22(br, dp) C22(br, dp)                                             \
                    EMITW(0x0B000000 | MRM(TPxx,    (br),    TDxx) | ADR)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tmm0    0x00  /* v0,  internal name for Xmm0 (in mmv) */
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
#define XmmE    0x0E, 0x00, EMPTY       /* v14 */

/* only for 128-bit instructions (save/restore in 256-bit header)
 * provided as an extension to common baseline of 15 registers */

#define XmmF    0x1E, 0x00, EMPTY       /* v30 */
#define XmmG    0x10, 0x00, EMPTY       /* v16 */
#define XmmH    0x11, 0x00, EMPTY       /* v17 */
#define XmmI    0x12, 0x00, EMPTY       /* v18 */
#define XmmJ    0x13, 0x00, EMPTY       /* v19 */
#define XmmK    0x14, 0x00, EMPTY       /* v20 */
#define XmmL    0x15, 0x00, EMPTY       /* v21 */
#define XmmM    0x16, 0x00, EMPTY       /* v22 */
#define XmmN    0x17, 0x00, EMPTY       /* v23 */
#define XmmO    0x18, 0x00, EMPTY       /* v24 */
#define XmmP    0x19, 0x00, EMPTY       /* v25 */
#define XmmQ    0x1A, 0x00, EMPTY       /* v26 */
#define XmmR    0x1B, 0x00, EMPTY       /* v27 */
#define XmmS    0x1C, 0x00, EMPTY       /* v28 */
#define XmmT    0x1D, 0x00, EMPTY       /* v29 */

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmix_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movrs_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))

#define movix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movix_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3C800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvix_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))

#define mmvix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))

#define mmvix_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3C800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andix_rr(XG, XS)                                                    \
        andix3rr(W(XG), W(XG), W(XS))

#define andix_ld(XG, MS, DS)                                                \
        andix3ld(W(XG), W(XG), W(MS), W(DS))

#define andix3rr(XD, XS, XT)                                                \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define andix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annix_rr(XG, XS)                                                    \
        annix3rr(W(XG), W(XG), W(XS))

#define annix_ld(XG, MS, DS)                                                \
        annix3ld(W(XG), W(XG), W(MS), W(DS))

#define annix3rr(XD, XS, XT)                                                \
        EMITW(0x4E601C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define annix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E601C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrix_rr(XG, XS)                                                    \
        orrix3rr(W(XG), W(XG), W(XS))

#define orrix_ld(XG, MS, DS)                                                \
        orrix3ld(W(XG), W(XG), W(MS), W(DS))

#define orrix3rr(XD, XS, XT)                                                \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornix_rr(XG, XS)                                                    \
        ornix3rr(W(XG), W(XG), W(XS))

#define ornix_ld(XG, MS, DS)                                                \
        ornix3ld(W(XG), W(XG), W(MS), W(DS))

#define ornix3rr(XD, XS, XT)                                                \
        EMITW(0x4EE01C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define ornix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE01C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorix_rr(XG, XS)                                                    \
        xorix3rr(W(XG), W(XG), W(XS))

#define xorix_ld(XG, MS, DS)                                                \
        xorix3ld(W(XG), W(XG), W(MS), W(DS))

#define xorix3rr(XD, XS, XT)                                                \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notix_rx(XG)                                                        \
        notix_rr(W(XG), W(XG))

#define notix_rr(XD, XS)                                                    \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XS), 0x00))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negis_rx(XG)                                                        \
        negis_rr(W(XG), W(XG))

#define negis_rr(XD, XS)                                                    \
        EMITW(0x6EA0F800 | MXM(REG(XD), REG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addis_rr(XG, XS)                                                    \
        addis3rr(W(XG), W(XG), W(XS))

#define addis_ld(XG, MS, DS)                                                \
        addis3ld(W(XG), W(XG), W(MS), W(DS))

#define addis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20D400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20D400 | MXM(REG(XD), REG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  adpis3rr
#define adpis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20D400 | MXM(REG(XD), REG(XS), REG(XT)))

#undef  adpis3ld
#define adpis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20D400 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subis_rr(XG, XS)                                                    \
        subis3rr(W(XG), W(XG), W(XS))

#define subis_ld(XG, MS, DS)                                                \
        subis3ld(W(XG), W(XG), W(MS), W(DS))

#define subis3rr(XD, XS, XT)                                                \
        EMITW(0x4EA0D400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0D400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulis_rr(XG, XS)                                                    \
        mulis3rr(W(XG), W(XG), W(XS))

#define mulis_ld(XG, MS, DS)                                                \
        mulis3ld(W(XG), W(XG), W(MS), W(DS))

#define mulis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20DC00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20DC00 | MXM(REG(XD), REG(XS), TmmM))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divis_rr(XG, XS)                                                    \
        divis3rr(W(XG), W(XG), W(XS))

#define divis_ld(XG, MS, DS)                                                \
        divis3ld(W(XG), W(XG), W(MS), W(DS))

#define divis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20FC00 | MXM(REG(XD), REG(XS), REG(XT)))

#define divis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20FC00 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqris_rr(XD, XS)                                                    \
        EMITW(0x6EA1F800 | MXM(REG(XD), REG(XS), 0x00))

#define sqris_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA1F800 | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITW(0x4EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E20FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITW(0x6EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E20DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EA0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minis_rr(XG, XS)                                                    \
        minis3rr(W(XG), W(XG), W(XS))

#define minis_ld(XG, MS, DS)                                                \
        minis3ld(W(XG), W(XG), W(MS), W(DS))

#define minis3rr(XD, XS, XT)                                                \
        EMITW(0x4EA0F400 | MXM(REG(XD), REG(XS), REG(XT)))

#define minis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0F400 | MXM(REG(XD), REG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxis_rr(XG, XS)                                                    \
        maxis3rr(W(XG), W(XG), W(XS))

#define maxis_ld(XG, MS, DS)                                                \
        maxis3ld(W(XG), W(XG), W(MS), W(DS))

#define maxis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20F400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20F400 | MXM(REG(XD), REG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqis_rr(XG, XS)                                                    \
        ceqis3rr(W(XG), W(XG), W(XS))

#define ceqis_ld(XG, MS, DS)                                                \
        ceqis3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneis_rr(XG, XS)                                                    \
        cneis3rr(W(XG), W(XG), W(XS))

#define cneis_ld(XG, MS, DS)                                                \
        cneis3ld(W(XG), W(XG), W(MS), W(DS))

#define cneis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cneis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltis_rr(XG, XS)                                                    \
        cltis3rr(W(XG), W(XG), W(XS))

#define cltis_ld(XG, MS, DS)                                                \
        cltis3ld(W(XG), W(XG), W(MS), W(DS))

#define cltis3rr(XD, XS, XT)                                                \
        EMITW(0x6EA0E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA0E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleis_rr(XG, XS)                                                    \
        cleis3rr(W(XG), W(XG), W(XS))

#define cleis_ld(XG, MS, DS)                                                \
        cleis3ld(W(XG), W(XG), W(MS), W(DS))

#define cleis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cleis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtis_rr(XG, XS)                                                    \
        cgtis3rr(W(XG), W(XG), W(XS))

#define cgtis_ld(XG, MS, DS)                                                \
        cgtis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtis3rr(XD, XS, XT)                                                \
        EMITW(0x6EA0E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA0E400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeis_rr(XG, XS)                                                    \
        cgeis3rr(W(XG), W(XG), W(XS))

#define cgeis_ld(XG, MS, DS)                                                \
        cgeis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20E400 | MXM(REG(XD), REG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_128     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128     0x04    /*  all satisfy the condition */

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4EB1B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask##32_128))                     \
        jezxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA19800 | MXM(REG(XD), REG(XS), 0x00))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA19800 | MXM(REG(XD), TmmM,    0x00))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA1B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1B800 | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA18800 | MXM(REG(XD), REG(XS), 0x00))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA18800 | MXM(REG(XD), TmmM,    0x00))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA1A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1A800 | MXM(REG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E219800 | MXM(REG(XD), REG(XS), 0x00))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E219800 | MXM(REG(XD), TmmM,    0x00))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E21B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21B800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E218800 | MXM(REG(XD), TmmM,    0x00))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21A800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        cvtin_rr(W(XD), W(XS))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        cvtin_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        EMITW(0x6EA19800 | MXM(REG(XD), REG(XS), 0x00))

#define rndis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA19800 | MXM(REG(XD), TmmM,    0x00))

#define cvtis_rr(XD, XS)                                                    \
        rndis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvtis_ld(XD, MS, DS)                                                \
        rndis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtin_rr(XD, XS)                                                    \
        EMITW(0x4E21D800 | MXM(REG(XD), REG(XS), 0x00))

#define cvtin_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21D800 | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvris_rr(XD, XS, mode)                                              \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addix_rr(XG, XS)                                                    \
        addix3rr(W(XG), W(XG), W(XS))

#define addix_ld(XG, MS, DS)                                                \
        addix3ld(W(XG), W(XG), W(MS), W(DS))

#define addix3rr(XD, XS, XT)                                                \
        EMITW(0x4EA08400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA08400 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subix_rr(XG, XS)                                                    \
        subix3rr(W(XG), W(XG), W(XS))

#define subix_ld(XG, MS, DS)                                                \
        subix3ld(W(XG), W(XG), W(MS), W(DS))

#define subix3rr(XD, XS, XT)                                                \
        EMITW(0x6EA08400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA08400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulix_rr(XG, XS)                                                    \
        mulix3rr(W(XG), W(XG), W(XS))

#define mulix_ld(XG, MS, DS)                                                \
        mulix3ld(W(XG), W(XG), W(MS), W(DS))

#define mulix3rr(XD, XS, XT)                                                \
        EMITW(0x4EA09C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA09C00 | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlix_ri(XG, IS)                                                    \
        shlix3ri(W(XG), W(XG), W(IS))

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix3ld(W(XG), W(XG), W(MS), W(DS))

#define shlix3ri(XD, XS, IT)                                                \
        EMITW(0x4F205400 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shlix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrix_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shrix3ri(W(XG), W(XG), W(IS))

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix3ld(W(XG), W(XG), W(MS), W(DS))

#define shrix3ri(XD, XS, IT)                                                \
        EMITW(0x4F200400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IT)) << 16)

#define shrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrin_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shrin3ri(W(XG), W(XG), W(IS))

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin3ld(W(XG), W(XG), W(MS), W(DS))

#define shrin3ri(XD, XS, IT)                                                \
        EMITW(0x4F200400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IT)) << 16)

#define shrin3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix3rr(W(XG), W(XG), W(XS))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix3ld(W(XG), W(XG), W(MS), W(DS))

#define svlix3rr(XD, XS, XT)                                                \
        EMITW(0x6EA04400 | MXM(REG(XD), REG(XS), REG(XT)))

#define svlix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA04400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix3rr(W(XG), W(XG), W(XS))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix3ld(W(XG), W(XG), W(MS), W(DS))

#define svrix3rr(XD, XS, XT)                                                \
        EMITW(0x6EA0B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XD), REG(XS), TmmM))

#define svrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin3rr(W(XG), W(XG), W(XS))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin3ld(W(XG), W(XG), W(MS), W(DS))

#define svrin3rr(XD, XS, XT)                                                \
        EMITW(0x6EA0B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XD), REG(XS), TmmM))

#define svrin3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3CC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*********   scalar single-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movrs_rr(XD, XS)                                                    \
        EMITW(0x5E040400 | MXM(REG(XD), REG(XS), 0x00))

#define movrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xBC400000 | MPM(REG(XD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movrs_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xBC000000 | MPM(REG(XS), MOD(MD), VAL(DD), B1(DD), P1(DD)))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addrs_rr(XG, XS)                                                    \
        addrs3rr(W(XG), W(XG), W(XS))

#define addrs_ld(XG, MS, DS)                                                \
        addrs3ld(W(XG), W(XG), W(MS), W(DS))

#define addrs3rr(XD, XS, XT)                                                \
        EMITW(0x1E202800 | MXM(REG(XD), REG(XS), REG(XT)))

#define addrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E202800 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subrs_rr(XG, XS)                                                    \
        subrs3rr(W(XG), W(XG), W(XS))

#define subrs_ld(XG, MS, DS)                                                \
        subrs3ld(W(XG), W(XG), W(MS), W(DS))

#define subrs3rr(XD, XS, XT)                                                \
        EMITW(0x1E203800 | MXM(REG(XD), REG(XS), REG(XT)))

#define subrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E203800 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulrs_rr(XG, XS)                                                    \
        mulrs3rr(W(XG), W(XG), W(XS))

#define mulrs_ld(XG, MS, DS)                                                \
        mulrs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulrs3rr(XD, XS, XT)                                                \
        EMITW(0x1E200800 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E200800 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divrs_rr(XG, XS)                                                    \
        divrs3rr(W(XG), W(XG), W(XS))

#define divrs_ld(XG, MS, DS)                                                \
        divrs3ld(W(XG), W(XG), W(MS), W(DS))

#define divrs3rr(XD, XS, XT)                                                \
        EMITW(0x1E201800 | MXM(REG(XD), REG(XS), REG(XT)))

#define divrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E201800 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
        EMITW(0x1E21C000 | MXM(REG(XD), REG(XS), 0x00))

#define sqrrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1E21C000 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
        EMITW(0x5EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsrs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x5E20FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1E200800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
        EMITW(0x7EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssrs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1E200800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x5EA0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1E200800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMITW(0x1F000000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1F000000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMITW(0x1F008000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1F008000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   scalar single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minrs_rr(XG, XS)                                                    \
        minrs3rr(W(XG), W(XG), W(XS))

#define minrs_ld(XG, MS, DS)                                                \
        minrs3ld(W(XG), W(XG), W(MS), W(DS))

#define minrs3rr(XD, XS, XT)                                                \
        EMITW(0x1E205800 | MXM(REG(XD), REG(XS), REG(XT)))

#define minrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E205800 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxrs_rr(XG, XS)                                                    \
        maxrs3rr(W(XG), W(XG), W(XS))

#define maxrs_ld(XG, MS, DS)                                                \
        maxrs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxrs3rr(XD, XS, XT)                                                \
        EMITW(0x1E204800 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x1E204800 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqrs_rr(XG, XS)                                                    \
        ceqrs3rr(W(XG), W(XG), W(XS))

#define ceqrs_ld(XG, MS, DS)                                                \
        ceqrs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqrs3rr(XD, XS, XT)                                                \
        EMITW(0x5E20E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x5E20E400 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cners_rr(XG, XS)                                                    \
        cners3rr(W(XG), W(XG), W(XS))

#define cners_ld(XG, MS, DS)                                                \
        cners3ld(W(XG), W(XG), W(MS), W(DS))

#define cners3rr(XD, XS, XT)                                                \
        EMITW(0x5E20E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cners3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x5E20E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltrs_rr(XG, XS)                                                    \
        cltrs3rr(W(XG), W(XG), W(XS))

#define cltrs_ld(XG, MS, DS)                                                \
        cltrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltrs3rr(XD, XS, XT)                                                \
        EMITW(0x7EA0E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7EA0E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clers_rr(XG, XS)                                                    \
        clers3rr(W(XG), W(XG), W(XS))

#define clers_ld(XG, MS, DS)                                                \
        clers3ld(W(XG), W(XG), W(MS), W(DS))

#define clers3rr(XD, XS, XT)                                                \
        EMITW(0x7E20E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define clers3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7E20E400 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtrs_rr(XG, XS)                                                    \
        cgtrs3rr(W(XG), W(XG), W(XS))

#define cgtrs_ld(XG, MS, DS)                                                \
        cgtrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtrs3rr(XD, XS, XT)                                                \
        EMITW(0x7EA0E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7EA0E400 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgers_rr(XG, XS)                                                    \
        cgers3rr(W(XG), W(XG), W(XS))

#define cgers_ld(XG, MS, DS)                                                \
        cgers3ld(W(XG), W(XG), W(MS), W(DS))

#define cgers3rr(XD, XS, XT)                                                \
        EMITW(0x7E20E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgers3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xBC400000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7E20E400 | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/**********************************   MODE   **********************************/
/******************************************************************************/

/************************   helper macros (FPU mode)   ************************/

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/POWER),
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

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#ifndef RT_RTARCH_A64_128X2V1_H
#undef  RT_128X2
#define RT_128X2  RT_128X1
#include "rtarch_a64_128x2v1.h"
#endif /* RT_RTARCH_A64_128X2V1_H */

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A32_128X1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

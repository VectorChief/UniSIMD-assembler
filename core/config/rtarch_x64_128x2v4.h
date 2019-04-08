/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_128X2V4_H
#define RT_RTARCH_X64_128X2V4_H

#include "rtarch_x32_128x2v4.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_128x2v4.h: Implementation of x86_64 fp64 SSE2/4 instruction pairs.
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

#if (RT_128X2 == 4)

/* mandatory escape prefix for some opcodes (must preceed rex) */
#define xF2                                                                 \
        EMITB(0xF2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmdx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmjx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movdx_rr(XD, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movdx_ld(XD, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define movdx_st(XS, MD, DD)                                                \
ADR ESC REX(0,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VYL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvdx_rr(XG, XS)                                                    \
        anddx_rr(W(XS), Xmm0)                                               \
        anndx_rr(Xmm0, W(XG))                                               \
        orrdx_rr(Xmm0, W(XS))                                               \
        movdx_rr(W(XG), Xmm0)

#define mmvdx_ld(XG, MS, DS)                                                \
        notdx_rx(Xmm0)                                                      \
        anddx_rr(W(XG), Xmm0)                                               \
        anndx_ld(Xmm0, W(MS), W(DS))                                        \
        orrdx_rr(W(XG), Xmm0)

#define mmvdx_st(XS, MG, DG)                                                \
        anddx_rr(W(XS), Xmm0)                                               \
        anndx_ld(Xmm0, W(MG), W(DG))                                        \
        orrdx_rr(Xmm0, W(XS))                                               \
        movdx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define anddx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define anddx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define anddx3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        anddx_rr(W(XD), W(XT))

#define anddx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        anddx_ld(W(XD), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define anndx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define anndx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define anndx3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        anndx_rr(W(XD), W(XT))

#define anndx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        anndx_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrdx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrdx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define orrdx3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        orrdx_rr(W(XD), W(XT))

#define orrdx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        orrdx_ld(W(XD), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define orndx_rr(XG, XS)                                                    \
        notdx_rx(W(XG))                                                     \
        orrdx_rr(W(XG), W(XS))

#define orndx_ld(XG, MS, DS)                                                \
        notdx_rx(W(XG))                                                     \
        orrdx_ld(W(XG), W(MS), W(DS))

#define orndx3rr(XD, XS, XT)                                                \
        notdx_rr(W(XD), W(XS))                                              \
        orrdx_rr(W(XD), W(XT))

#define orndx3ld(XD, XS, MT, DT)                                            \
        notdx_rr(W(XD), W(XS))                                              \
        orrdx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xordx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xordx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define xordx3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        xordx_rr(W(XD), W(XT))

#define xordx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        xordx_ld(W(XD), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notdx_rx(XG)                                                        \
        anndx_ld(W(XG), Mebp, inf_GPC07)

#define notdx_rr(XD, XS)                                                    \
        movdx_rr(W(XD), W(XS))                                              \
        notdx_rx(W(XD))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negds_rx(XG)                                                        \
        xordx_ld(W(XG), Mebp, inf_GPC06_64)

#define negds_rr(XD, XS)                                                    \
        movdx_rr(W(XD), W(XS))                                              \
        negds_rx(W(XD))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define addds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        addds_rr(W(XD), W(XT))

#define addds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        addds_ld(W(XD), W(MT), W(DT))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_SIMD_COMPAT_SSE < 4)

#undef  adpds_rx
#define adpds_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        addts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))

#endif /* RT_SIMD_COMPAT_SSE < 4 */

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define subds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        subds_rr(W(XD), W(XT))

#define subds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        subds_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define mulds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        mulds_rr(W(XD), W(XT))

#define mulds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        mulds_ld(W(XD), W(MT), W(DT))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define divds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        divds_rr(W(XD), W(XT))

#define divds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        divds_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrds_rr(XD, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrds_ld(XD, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceds_rr(XD, XS)                                                    \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_ld(W(XD), Mebp, inf_SCR01(0))

#define rcsds_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseds_rr(XD, XS)                                                    \
        sqrds_rr(W(XD), W(XS))                                              \
        movdx_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divds_ld(W(XD), Mebp, inf_SCR01(0))

#define rssds_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmads_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_rr(W(XS), W(XT))                                              \
        addds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmads_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_ld(W(XS), W(MT), W(DT))                                       \
        addds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmads_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmads_rx(W(XG))

#define fmads_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmads_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmads_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmads_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmads_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmads_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmads_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        movdx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x18))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x10))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movdx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsds_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_rr(W(XS), W(XT))                                              \
        subds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulds_ld(W(XS), W(MT), W(DT))                                       \
        subds_rr(W(XG), W(XS))                                              \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsds_rr(XG, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsds_rx(W(XG))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsds_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsds_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsds_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XS), W(MT), W(DT))                                       \
        movdx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movdx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsds_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsds_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        movdx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrzs_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x18))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x10))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movdx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define minds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        minds_rr(W(XD), W(XT))

#define minds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        minds_ld(W(XD), W(MT), W(DT))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define maxds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        maxds_rr(W(XD), W(XT))

#define maxds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        maxds_ld(W(XD), W(MT), W(DT))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x00))

#define ceqds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        ceqds_rr(W(XD), W(XT))

#define ceqds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        ceqds_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x04))

#define cneds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        cneds_rr(W(XD), W(XT))

#define cneds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        cneds_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x01))

#define cltds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        cltds_rr(W(XD), W(XT))

#define cltds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        cltds_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x02))

#define cleds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        cleds_rr(W(XD), W(XT))

#define cleds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        cleds_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x06))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x06))

#define cgtds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        cgtds_rr(W(XD), W(XT))

#define cgtds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        cgtds_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeds_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeds_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x05))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x05))

#define cgeds3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        cgeds_rr(W(XD), W(XT))

#define cgeds3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        cgeds_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_256    0x0F     /*  all satisfy the condition */

#define mkjdx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        REX(0,             0) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(1,             0) EMITB(0x8B)                                   \
        MRM(0x07,       0x03, 0x00)                                         \
        REX(0,             1) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##64_256 >> 3) << 1)))   \
        MRM(0x00,       0x03, 0x07)                                         \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##64_256))                     \
        jeqxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
        cvzds_rr(W(XD), W(XS))                                              \
        cvndn_rr(W(XD), W(XD))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
        cvzds_ld(W(XD), W(MS), W(DS))                                       \
        cvndn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x03))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

#define cvzds_rr(XD, XS)     /* round towards zero */                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x18))                                    \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvzds_ld(XD, MS, DS) /* round towards zero */                       \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
        cvpds_rr(W(XD), W(XS))                                              \
        cvndn_rr(W(XD), W(XD))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpds_ld(W(XD), W(MS), W(DS))                                       \
        cvndn_rr(W(XD), W(XD))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnds_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnds_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x02))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        rnpds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
        cvmds_rr(W(XD), W(XS))                                              \
        cvndn_rr(W(XD), W(XD))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmds_ld(W(XD), W(MS), W(DS))                                       \
        cvndn_rr(W(XD), W(XD))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnds_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnds_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x01))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        rnmds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnnds_rr(XD, XS)     /* round towards near */                       \
        cvnds_rr(W(XD), W(XS))                                              \
        cvndn_rr(W(XD), W(XD))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
        cvnds_ld(W(XD), W(MS), W(DS))                                       \
        cvndn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnnds_rr(XD, XS)     /* round towards near */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x00))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

#define cvnds_rr(XD, XS)     /* round towards near */                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x18))                                    \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnds_ld(XD, MS, DS) /* round towards near */                       \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvnds_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndn_rr(XD, XS)     /* round towards near */                       \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x18))                                    \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvndn_ld(XD, MS, DS) /* round towards near */                       \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvndn_rr(W(XD), W(XD))

#if (RT_SIMD_COMPAT_SSE < 4)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
        cvtds_rr(W(XD), W(XS))                                              \
        cvndn_rr(W(XD), W(XD))

#define rndds_ld(XD, MS, DS)                                                \
        cvtds_ld(W(XD), W(MS), W(DS))                                       \
        cvndn_rr(W(XD), W(XD))

#define cvtds_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnds_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtds_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvtds_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndds_ld(XD, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x04))

#define cvtds_rr(XD, XS)                                                    \
        rndds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvtds_ld(XD, MS, DS)                                                \
        rndds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdn_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvndn_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtdn_ld(XD, MS, DS)                                                \
        movdx_ld(W(XD), W(MS), W(DS))                                       \
        cvtdn_rr(W(XD), W(XD))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnrds_rr(XD, XS, mode)                                              \
        cvrds_rr(W(XD), W(XS), mode)                                        \
        cvndn_rr(W(XD), W(XD))

#define cvrds_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtds_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnrds_rr(XD, XS, mode)                                              \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrds_rr(XD, XS, mode)                                              \
        rnrds_rr(W(XD), W(XS), mode)                                        \
        cvzds_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define adddx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adddx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define adddx3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        adddx_rr(W(XD), W(XT))

#define adddx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        adddx_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subdx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subdx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define subdx3rr(XD, XS, XT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        subdx_rr(W(XD), W(XT))

#define subdx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        subdx_ld(W(XD), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shldx_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))                        \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xF3)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xF3)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define shldx3ri(XD, XS, IT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        shldx_ri(W(XD), W(IT))

#define shldx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        shldx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdx_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))                        \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xD3)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xD3)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define shrdx3ri(XD, XS, IT)                                                \
        movdx_rr(W(XD), W(XS))                                              \
        shrdx_ri(W(XD), W(IT))

#define shrdx3ld(XD, XS, MT, DT)                                            \
        movdx_rr(W(XD), W(XS))                                              \
        shrdx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdn_ri(XG, IS)                                                    \
        shrdn3ri(W(XG), W(XG), W(IS))

#define shrdn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdn3ri(XD, XS, IT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x10), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x18), W(IT))                             \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MT), W(DT))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx3rr(W(XG), W(XG), W(XS))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx3ld(W(XG), W(XG), W(MS), W(DS))

#define svldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define svldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx3rr(W(XG), W(XG), W(XS))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn3rr(W(XG), W(XG), W(XS))

#define svrdn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdn3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_128X2V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

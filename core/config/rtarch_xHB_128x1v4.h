/******************************************************************************/
/* Copyright (c) 2013-2022 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_128X1V4_H
#define RT_RTARCH_XHB_128X1V4_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_128x1v4.h: Implementation of x86_64 half+byte SSE2/4 instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdm*_rx - applies [cmd] to var-len packed SIMD: [r]egister (one operand)
 * cmdm*_rr - applies [cmd] to var-len packed SIMD: [r]egister from [r]egister
 *
 * cmdm*_rm - applies [cmd] to var-len packed SIMD: [r]egister from [m]emory
 * cmdm*_ld - applies [cmd] to var-len packed SIMD: as above (friendly alias)
 *
 * cmdg*_** - applies [cmd] to 16-bit elements SIMD args, packed-128-bit
 * cmdgb_** - applies [cmd] to u-char elements SIMD args, packed-128-bit
 * cmdgc_** - applies [cmd] to s-char elements SIMD args, packed-128-bit
 *
 * cmda*_** - applies [cmd] to 16-bit elements SIMD args, packed-256-bit
 * cmdab_** - applies [cmd] to u-char elements SIMD args, packed-256-bit
 * cmdac_** - applies [cmd] to s-char elements SIMD args, packed-256-bit
 *
 * cmdn*_** - applies [cmd] to 16-bit elements ELEM args, scalar-fp-only
 * cmdh*_** - applies [cmd] to 16-bit elements BASE args, BASE-regs-only
 * cmdb*_** - applies [cmd] to  8-bit elements BASE args, BASE-regs-only
 *
 * cmd*x_** - applies [cmd] to SIMD/BASE unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to SIMD/BASE   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to SIMD/ELEM floating point   args, [s] - scalable
 *
 * The cmdm*_** (rtconf.h) instructions are intended for SPMD programming model
 * and simultaneously support 16/8-bit data elements (int, fp16 on ARM and x86).
 * In this model data paths are fixed-width, BASE and SIMD data elements are
 * width-compatible, code path divergence is handled via mkj**_** pseudo-ops.
 * Matching 16/8-bit BASE subsets cmdh* / cmdb* are defined in rtarch_*HB.h.
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
 * should propagate QNaNs unchanged, however this behavior hasn't been tested.
 *
 * Note, that instruction subsets operating on vectors of different length
 * may support different number of SIMD registers, therefore mixing them
 * in the same code needs to be done with register awareness in mind.
 * For example, AVX-512 supports 32 SIMD registers, while AVX2 only has 16,
 * as does 256-bit paired subset on ARMv8, while 128-bit and SVE have 32.
 * These numbers should be consistent across architectures if properly
 * mapped to SIMD target mask presented in rtzero.h (compatibility layer).
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * XD - SIMD register serving as destination only, if present
 * XG - SIMD register serving as destination and first source
 * XS - SIMD register serving as second source (first if any)
 * XT - SIMD register serving as third source (second if any)
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and first source
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

#if (RT_128X1 == 4)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgx_ld(XD, MS, DS)                                                \
    ADR REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgx_st(XS, MD, DD)                                                \
    ADR REX(RXB(XS), RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgx_rr(XG, XS)                                                    \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_rr(Xmm0, W(XG))                                               \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_rr(W(XG), Xmm0)

#define mmvgx_ld(XG, MS, DS)                                                \
        notgx_rx(Xmm0)                                                      \
        andgx_rr(W(XG), Xmm0)                                               \
        anngx_ld(Xmm0, W(MS), W(DS))                                        \
        orrgx_rr(W(XG), Xmm0)

#define mmvgx_st(XS, MG, DG)                                                \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_ld(Xmm0, W(MG), W(DG))                                        \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDB)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        andgx_rr(W(XD), W(XT))

#define andgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        andgx_ld(W(XD), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anngx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define anngx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDF)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define anngx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        anngx_rr(W(XD), W(XT))

#define anngx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        anngx_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEB)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        orrgx_rr(W(XD), W(XT))

#define orrgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        orrgx_ld(W(XD), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orngx_rr(XG, XS)                                                    \
        notgx_rx(W(XG))                                                     \
        orrgx_rr(W(XG), W(XS))

#define orngx_ld(XG, MS, DS)                                                \
        notgx_rx(W(XG))                                                     \
        orrgx_ld(W(XG), W(MS), W(DS))

#define orngx3rr(XD, XS, XT)                                                \
        notgx_rr(W(XD), W(XS))                                              \
        orrgx_rr(W(XD), W(XT))

#define orngx3ld(XD, XS, MT, DT)                                            \
        notgx_rr(W(XD), W(XS))                                              \
        orrgx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEF)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_rr(W(XD), W(XT))

#define xorgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_ld(W(XD), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        anngx_ld(W(XG), Mebp, inf_GPC07)

#define notgx_rr(XD, XS)                                                    \
        movgx_rr(W(XD), W(XS))                                              \
        notgx_rx(W(XD))

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xFD)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        addgx_rr(W(XD), W(XT))

#define addgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        addgx_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDD)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define adsgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        adsgx_rr(W(XD), W(XT))

#define adsgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        adsgx_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xED)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define adsgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        adsgn_rr(W(XD), W(XT))

#define adsgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        adsgn_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xF9)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        subgx_rr(W(XD), W(XT))

#define subgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        subgx_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD9)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbsgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgx_rr(W(XD), W(XT))

#define sbsgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgx_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xE9)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbsgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgn_rr(W(XD), W(XT))

#define sbsgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgn_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD5)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mulgx_rr(W(XD), W(XT))

#define mulgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mulgx_ld(W(XD), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xF1)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shlgx3ri(XD, XS, IT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        shlgx_ri(W(XD), W(IT))

#define shlgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        shlgx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD1)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrgx3ri(XD, XS, IT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        shrgx_ri(W(XD), W(IT))

#define shrgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        shrgx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x71)                       \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xE1)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrgn3ri(XD, XS, IT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        shrgn_ri(W(XD), W(IT))

#define shrgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        shrgn_ld(W(XD), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx3rr(W(XG), W(XG), W(XS))

#define svlgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgx_rx(W(XD))

#define svlgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlgx_rx(W(XD))

#define svlgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx3rr(W(XG), W(XG), W(XS))

#define svrgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgx_rx(W(XD))

#define svrgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgx_rx(W(XD))

#define svrgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn3rr(W(XG), W(XG), W(XS))

#define svrgn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgn3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgn_rx(W(XD))

#define svrgn3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgn_rx(W(XD))

#define svrgn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed half-precision integer compare   ******************/

#if (RT_SIMD_COMPAT_SSE < 4)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
        mingx3rr(W(XG), W(XG), W(XS))

#define mingx_ld(XG, MS, DS)                                                \
        mingx3ld(W(XG), W(XG), W(MS), W(DS))

#define mingx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mingx_rx(W(XD))

#define mingx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mingx_rx(W(XD))

#define mingx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x73) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        stack_ld(Reax)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR02(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
        maxgx3rr(W(XG), W(XG), W(XS))

#define maxgx_ld(XG, MS, DS)                                                \
        maxgx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxgx_rx(W(XD))

#define maxgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxgx_rx(W(XD))

#define maxgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmphx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x76) EMITB(0x08 + x67)                                       \
        movhx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        stack_ld(Reax)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR02(0))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mingx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3A)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mingx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mingx_rr(W(XD), W(XT))

#define mingx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mingx_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3E)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        maxgx_rr(W(XD), W(XT))

#define maxgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        maxgx_ld(W(XD), W(MT), W(DT))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mingn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEA)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mingn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mingn_rr(W(XD), W(XT))

#define mingn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mingn_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEE)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        maxgn_rr(W(XD), W(XT))

#define maxgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        maxgn_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define ceqgx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x75)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define ceqgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        ceqgx_rr(W(XD), W(XT))

#define ceqgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        ceqgx_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegx_rr(XG, XS)                                                    \
        ceqgx_rr(W(XG), W(XS))                                              \
        notgx_rx(W(XG))

#define cnegx_ld(XG, MS, DS)                                                \
        ceqgx_ld(W(XG), W(MS), W(DS))                                       \
        notgx_rx(W(XG))

#define cnegx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cnegx_rr(W(XD), W(XT))

#define cnegx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cnegx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgx_rr(XG, XS)                                                    \
        mingx_rr(W(XG), W(XS))                                              \
        cnegx_rr(W(XG), W(XS))

#define cltgx_ld(XG, MS, DS)                                                \
        mingx_ld(W(XG), W(MS), W(DS))                                       \
        cnegx_ld(W(XG), W(MS), W(DS))

#define cltgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cltgx_rr(W(XD), W(XT))

#define cltgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cltgx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgn_rr(XG, XS)                                                    \
        mingn_rr(W(XG), W(XS))                                              \
        cnegx_rr(W(XG), W(XS))

#define cltgn_ld(XG, MS, DS)                                                \
        mingn_ld(W(XG), W(MS), W(DS))                                       \
        cnegx_ld(W(XG), W(MS), W(DS))

#define cltgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cltgn_rr(W(XD), W(XT))

#define cltgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cltgn_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegx_rr(XG, XS)                                                    \
        maxgx_rr(W(XG), W(XS))                                              \
        ceqgx_rr(W(XG), W(XS))

#define clegx_ld(XG, MS, DS)                                                \
        maxgx_ld(W(XG), W(MS), W(DS))                                       \
        ceqgx_ld(W(XG), W(MS), W(DS))

#define clegx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        clegx_rr(W(XD), W(XT))

#define clegx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        clegx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegn_rr(XG, XS)                                                    \
        cgtgn_rr(W(XG), W(XS))                                              \
        notgx_rx(W(XG))

#define clegn_ld(XG, MS, DS)                                                \
        cgtgn_ld(W(XG), W(MS), W(DS))                                       \
        notgx_rx(W(XG))

#define clegn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        clegn_rr(W(XD), W(XT))

#define clegn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        clegn_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgx_rr(XG, XS)                                                    \
        maxgx_rr(W(XG), W(XS))                                              \
        cnegx_rr(W(XG), W(XS))

#define cgtgx_ld(XG, MS, DS)                                                \
        maxgx_ld(W(XG), W(MS), W(DS))                                       \
        cnegx_ld(W(XG), W(MS), W(DS))

#define cgtgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgx_rr(W(XD), W(XT))

#define cgtgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgx_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgn_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cgtgn_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x65)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cgtgn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgn_rr(W(XD), W(XT))

#define cgtgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgn_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegx_rr(XG, XS)                                                    \
        mingx_rr(W(XG), W(XS))                                              \
        ceqgx_rr(W(XG), W(XS))

#define cgegx_ld(XG, MS, DS)                                                \
        mingx_ld(W(XG), W(MS), W(DS))                                       \
        ceqgx_ld(W(XG), W(MS), W(DS))

#define cgegx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgegx_rr(W(XD), W(XT))

#define cgegx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgegx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegn_rr(XG, XS)                                                    \
        mingn_rr(W(XG), W(XS))                                              \
        ceqgx_rr(W(XG), W(XS))

#define cgegn_ld(XG, MS, DS)                                                \
        mingn_ld(W(XG), W(MS), W(DS))                                       \
        ceqgx_ld(W(XG), W(MS), W(DS))

#define cgegn3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgegn_rr(W(XD), W(XT))

#define cgegn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgegn_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_128    0x0F     /*  all satisfy the condition */

#define mkjgx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        REX(0,       RXB(XS)) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        shlix_ri(W(XS), IB(16))                                             \
        REX(1,       RXB(XS)) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x07,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##16_128 & 0x1) << 1)))  \
        MRM(0x00,       0x03, 0x07)                                         \
        movgx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##16_128))                     \
        jeqxx_lb(lb)

/****************   packed byte-precision generic move/logic   ****************/

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgb_rr(XG, XS)                                                    \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_rr(Xmm0, W(XG))                                               \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_rr(W(XG), Xmm0)

#define mmvgb_ld(XG, MS, DS)                                                \
        notgx_rx(Xmm0)                                                      \
        andgx_rr(W(XG), Xmm0)                                               \
        anngx_ld(Xmm0, W(MS), W(DS))                                        \
        orrgx_rr(W(XG), Xmm0)

#define mmvgb_st(XS, MG, DG)                                                \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_ld(Xmm0, W(MG), W(DG))                                        \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_st(Xmm0, W(MG), W(DG))

/* move/logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgb_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xFC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addgb_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xFC)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        addgb_rr(W(XD), W(XT))

#define addgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        addgb_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgb_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgb_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDC)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define adsgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        adsgb_rr(W(XD), W(XT))

#define adsgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        adsgb_ld(W(XD), W(MT), W(DT))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgc_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xEC)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgc_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xEC)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define adsgc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        adsgc_rr(W(XD), W(XT))

#define adsgc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        adsgc_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgb_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xF8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subgb_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xF8)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        subgb_rr(W(XD), W(XT))

#define subgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        subgb_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgb_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xD8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgb_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD8)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbsgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgb_rr(W(XD), W(XT))

#define sbsgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgb_ld(W(XD), W(MT), W(DT))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgc_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xE8)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgc_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xE8)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbsgc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgc_rr(W(XD), W(XT))

#define sbsgc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        sbsgc_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgb_rr(XG, XS)                                                    \
        mulgb3rr(W(XG), W(XG), W(XS))

#define mulgb_ld(XG, MS, DS)                                                \
        mulgb3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgb3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulgb_rx(W(XD))

#define mulgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulgb_rx(W(XD))

#define mulgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x01))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x01))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x02))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x02))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x03))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x03))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x05))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x05))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x06))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x06))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x07))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x07))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x09))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x09))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0F))                              \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgb_ri(XG, IS)                                                    \
        shlgb3ri(W(XG), W(XG), W(IS))

#define shlgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgb3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgb_xx() /* not portable, do not use outside */                   \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgb_ri(XG, IS)                                                    \
        shrgb3ri(W(XG), W(XG), W(IS))

#define shrgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgb3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgb_xx() /* not portable, do not use outside */                   \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgc_ri(XG, IS)                                                    \
        shrgc3ri(W(XG), W(XG), W(IS))

#define shrgc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgc3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgc3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgc_xx() /* not portable, do not use outside */                   \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgb3rr(W(XG), W(XG), W(XS))

#define svlgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgb3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgb_rx(W(XD))

#define svlgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlgb_rx(W(XD))

#define svlgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgb3rr(W(XG), W(XG), W(XS))

#define svrgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgb3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgb_rx(W(XD))

#define svrgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgb_rx(W(XD))

#define svrgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgc3rr(W(XG), W(XG), W(XS))

#define svrgc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgc3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgc_rx(W(XD))

#define svrgc3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgc_rx(W(XD))

#define svrgc_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingb_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mingb_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDA)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mingb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mingb_rr(W(XD), W(XT))

#define mingb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mingb_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgb_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xDE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxgb_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xDE)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        maxgb_rr(W(XD), W(XT))

#define maxgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        maxgb_ld(W(XD), W(MT), W(DT))

#if (RT_SIMD_COMPAT_SSE < 4)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingc_rr(XG, XS)                                                    \
        mingc3rr(W(XG), W(XG), W(XS))

#define mingc_ld(XG, MS, DS)                                                \
        mingc3ld(W(XG), W(XG), W(MS), W(DS))

#define mingc3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mingc_rx(W(XD))

#define mingc3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mingc_rx(W(XD))

#define mingc_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x01))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x01))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x01))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x03))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x03))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x03))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x05))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x05))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x05))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x07))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x07))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x07))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x09))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x09))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x09))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0B))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0B))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0B))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0D))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0D))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0D))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0F))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0F))                              \
        EMITB(0x7D) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0F))                              \
        stack_ld(Reax)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR02(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgc_rr(XG, XS)                                                    \
        maxgc3rr(W(XG), W(XG), W(XS))

#define maxgc_ld(XG, MS, DS)                                                \
        maxgc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgc3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxgc_rx(W(XD))

#define maxgc3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxgc_rx(W(XD))

#define maxgc_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Reax)                                                      \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x01))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x01))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x01))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x02))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x02))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x03))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x03))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x03))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x04))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x04))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x05))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x05))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x05))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x06))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x06))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x07))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x07))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x07))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x09))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x09))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x09))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0A))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0A))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0B))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0B))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0B))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0C))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0C))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0D))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0D))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0D))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0E))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0E))                              \
        movbx_ld(Reax,  Mebp, inf_SCR01(0x0F))                              \
        cmpbx_rm(Reax,  Mebp, inf_SCR02(0x0F))                              \
        EMITB(0x7E) EMITB(0x07 + x67)                                       \
        movbx_st(Reax,  Mebp, inf_SCR02(0x0F))                              \
        stack_ld(Reax)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR02(0))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingc_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x38)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mingc_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x38)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mingc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        mingc_rr(W(XD), W(XT))

#define mingc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        mingc_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgc_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3C)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxgc_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x3C)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxgc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        maxgc_rr(W(XD), W(XT))

#define maxgc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        maxgc_ld(W(XD), W(MT), W(DT))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgb_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x74)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define ceqgb_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x74)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define ceqgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        ceqgb_rr(W(XD), W(XT))

#define ceqgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        ceqgb_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegb_rr(XG, XS)                                                    \
        ceqgb_rr(W(XG), W(XS))                                              \
        notgx_rx(W(XG))

#define cnegb_ld(XG, MS, DS)                                                \
        ceqgb_ld(W(XG), W(MS), W(DS))                                       \
        notgx_rx(W(XG))

#define cnegb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cnegb_rr(W(XD), W(XT))

#define cnegb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cnegb_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgb_rr(XG, XS)                                                    \
        mingb_rr(W(XG), W(XS))                                              \
        cnegb_rr(W(XG), W(XS))

#define cltgb_ld(XG, MS, DS)                                                \
        mingb_ld(W(XG), W(MS), W(DS))                                       \
        cnegb_ld(W(XG), W(MS), W(DS))

#define cltgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cltgb_rr(W(XD), W(XT))

#define cltgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cltgb_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgc_rr(XG, XS)                                                    \
        mingc_rr(W(XG), W(XS))                                              \
        cnegb_rr(W(XG), W(XS))

#define cltgc_ld(XG, MS, DS)                                                \
        mingc_ld(W(XG), W(MS), W(DS))                                       \
        cnegb_ld(W(XG), W(MS), W(DS))

#define cltgc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cltgc_rr(W(XD), W(XT))

#define cltgc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cltgc_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegb_rr(XG, XS)                                                    \
        maxgb_rr(W(XG), W(XS))                                              \
        ceqgb_rr(W(XG), W(XS))

#define clegb_ld(XG, MS, DS)                                                \
        maxgb_ld(W(XG), W(MS), W(DS))                                       \
        ceqgb_ld(W(XG), W(MS), W(DS))

#define clegb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        clegb_rr(W(XD), W(XT))

#define clegb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        clegb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegc_rr(XG, XS)                                                    \
        cgtgc_rr(W(XG), W(XS))                                              \
        notgx_rx(W(XG))

#define clegc_ld(XG, MS, DS)                                                \
        cgtgc_ld(W(XG), W(MS), W(DS))                                       \
        notgx_rx(W(XG))

#define clegc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        clegc_rr(W(XD), W(XT))

#define clegc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        clegc_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgb_rr(XG, XS)                                                    \
        maxgb_rr(W(XG), W(XS))                                              \
        cnegb_rr(W(XG), W(XS))

#define cgtgb_ld(XG, MS, DS)                                                \
        maxgb_ld(W(XG), W(MS), W(DS))                                       \
        cnegb_ld(W(XG), W(MS), W(DS))

#define cgtgb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgb_rr(W(XD), W(XT))

#define cgtgb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgb_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgc_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x64)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cgtgc_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x64)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cgtgc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgc_rr(W(XD), W(XT))

#define cgtgc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgtgc_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegb_rr(XG, XS)                                                    \
        mingb_rr(W(XG), W(XS))                                              \
        ceqgb_rr(W(XG), W(XS))

#define cgegb_ld(XG, MS, DS)                                                \
        mingb_ld(W(XG), W(MS), W(DS))                                       \
        ceqgb_ld(W(XG), W(MS), W(DS))

#define cgegb3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgegb_rr(W(XD), W(XT))

#define cgegb3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgegb_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegc_rr(XG, XS)                                                    \
        mingc_rr(W(XG), W(XS))                                              \
        ceqgb_rr(W(XG), W(XS))

#define cgegc_ld(XG, MS, DS)                                                \
        mingc_ld(W(XG), W(MS), W(DS))                                       \
        ceqgb_ld(W(XG), W(MS), W(DS))

#define cgegc3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        cgegc_rr(W(XD), W(XT))

#define cgegc3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        cgegc_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_128    0x0F     /*  all satisfy the condition */

#define bsnix_rx(XS, mask) /* not portable, do not use outside */           \
        REX(1,       RXB(XS)) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x07,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##08_128 & 0x1) << 1)))  \
        MRM(0x00,       0x03, 0x07)

#define mkjgb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        REX(0,       RXB(XS)) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        shlix_ri(W(XS), IB(8))                                              \
        bsnix_rx(W(XS), mask)                                               \
        shlix_ri(W(XS), IB(8))                                              \
        bsnix_rx(W(XS), mask)                                               \
        shlix_ri(W(XS), IB(8))                                              \
        bsnix_rx(W(XS), mask)                                               \
        movgx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##08_128))                     \
        jeqxx_lb(lb)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_128X1V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

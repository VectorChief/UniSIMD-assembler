/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_128X1V2_H
#define RT_RTARCH_XHB_128X1V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_128x1v2.h: Implementation of x86_64 half+byte AVX512VL/BW ops.
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

#if (RT_128X1 == 2)

#define ck1gx_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVW(0,       RXB(XS),    0x00, 0, 2, 2) EMITB(0x29)                 \
        MRM(0x01,    MOD(XS), REG(XS))

#define ck1gb_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(0,       RXB(XS),    0x00, 0, 2, 2) EMITB(0x29)                 \
        MRM(0x01,    MOD(XS), REG(XS))

#define mz1gx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVW(RXB(XD),       0,    0x00, 0, 2, 2) EMITB(0x28)                 \
        MRM(REG(XD),    0x03,    0x01)

#define mz1gb_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVX(RXB(XD),       0,    0x00, 0, 2, 2) EMITB(0x28)                 \
        MRM(REG(XD),    0x03,    0x01)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgx_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgx_st(XS, MD, DD)                                                \
    ADR EVX(RXB(XS), RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgx_rr(XG, XS)                                                    \
        ck1gx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(RXB(XG), RXB(XS),    0x00, 0, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvgx_ld(XG, MS, DS)                                                \
        ck1gx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XG), RXB(MS),    0x00, 0, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvgx_st(XS, MG, DG)                                                \
        ck1gx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XS), RXB(MG),    0x00, 0, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
        andgx3rr(W(XG), W(XG), W(XS))

#define andgx_ld(XG, MS, DS)                                                \
        andgx3ld(W(XG), W(XG), W(MS), W(DS))

#define andgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anngx_rr(XG, XS)                                                    \
        anngx3rr(W(XG), W(XG), W(XS))

#define anngx_ld(XG, MS, DS)                                                \
        anngx3ld(W(XG), W(XG), W(MS), W(DS))

#define anngx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anngx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrgx_rr(XG, XS)                                                    \
        orrgx3rr(W(XG), W(XG), W(XS))

#define orrgx_ld(XG, MS, DS)                                                \
        orrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

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
        xorgx3rr(W(XG), W(XG), W(XS))

#define xorgx_ld(XG, MS, DS)                                                \
        xorgx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        notgx_rr(W(XG), W(XG))

#define notgx_rr(XD, XS)                                                    \
        anngx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
        addgx3rr(W(XG), W(XG), W(XS))

#define addgx_ld(XG, MS, DS)                                                \
        addgx3ld(W(XG), W(XG), W(MS), W(DS))

#define addgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgx_rr(XG, XS)                                                    \
        adsgx3rr(W(XG), W(XG), W(XS))

#define adsgx_ld(XG, MS, DS)                                                \
        adsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgn_rr(XG, XS)                                                    \
        adsgn3rr(W(XG), W(XG), W(XS))

#define adsgn_ld(XG, MS, DS)                                                \
        adsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgn3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
        subgx3rr(W(XG), W(XG), W(XS))

#define subgx_ld(XG, MS, DS)                                                \
        subgx3ld(W(XG), W(XG), W(MS), W(DS))

#define subgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgx_rr(XG, XS)                                                    \
        sbsgx3rr(W(XG), W(XG), W(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
        sbsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgn_rr(XG, XS)                                                    \
        sbsgn3rr(W(XG), W(XG), W(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
        sbsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgn3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
        mulgx3rr(W(XG), W(XG), W(XS))

#define mulgx_ld(XG, MS, DS)                                                \
        mulgx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)                                                    \
        shlgx3ri(W(XG), W(XG), W(IS))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgx3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), 0, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)                                                    \
        shrgx3ri(W(XG), W(XG), W(IS))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgx3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), 0, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)                                                    \
        shrgn3ri(W(XG), W(XG), W(IS))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgn3ri(XD, XS, IT)                                                \
        EVX(0,       RXB(XS), REN(XD), 0, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrgn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx3rr(W(XG), W(XG), W(XS))

#define svlgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlgx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x12)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx3rr(W(XG), W(XG), W(XS))

#define svrgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgx3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrgx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x10)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn3rr(W(XG), W(XG), W(XS))

#define svrgn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgn3rr(XD, XS, XT)                                                \
        EVW(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrgn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x11)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
        mingx3rr(W(XG), W(XG), W(XS))

#define mingx_ld(XG, MS, DS)                                                \
        mingx3ld(W(XG), W(XG), W(MS), W(DS))

#define mingx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingn_rr(XG, XS)                                                    \
        mingn3rr(W(XG), W(XG), W(XS))

#define mingn_ld(XG, MS, DS)                                                \
        mingn3ld(W(XG), W(XG), W(MS), W(DS))

#define mingn3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
        maxgx3rr(W(XG), W(XG), W(XS))

#define maxgx_ld(XG, MS, DS)                                                \
        maxgx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgx3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgx3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgn_rr(XG, XS)                                                    \
        maxgn3rr(W(XG), W(XG), W(XS))

#define maxgn_ld(XG, MS, DS)                                                \
        maxgn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgn3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgn3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgx_rr(XG, XS)                                                    \
        ceqgx3rr(W(XG), W(XG), W(XS))

#define ceqgx_ld(XG, MS, DS)                                                \
        ceqgx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define ceqgx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegx_rr(XG, XS)                                                    \
        cnegx3rr(W(XG), W(XG), W(XS))

#define cnegx_ld(XG, MS, DS)                                                \
        cnegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cnegx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgx_rr(XG, XS)                                                    \
        cltgx3rr(W(XG), W(XG), W(XS))

#define cltgx_ld(XG, MS, DS)                                                \
        cltgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cltgx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgn_rr(XG, XS)                                                    \
        cltgn3rr(W(XG), W(XG), W(XS))

#define cltgn_ld(XG, MS, DS)                                                \
        cltgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cltgn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegx_rr(XG, XS)                                                    \
        clegx3rr(W(XG), W(XG), W(XS))

#define clegx_ld(XG, MS, DS)                                                \
        clegx3ld(W(XG), W(XG), W(MS), W(DS))

#define clegx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define clegx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegn_rr(XG, XS)                                                    \
        clegn3rr(W(XG), W(XG), W(XS))

#define clegn_ld(XG, MS, DS)                                                \
        clegn3ld(W(XG), W(XG), W(MS), W(DS))

#define clegn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define clegn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgx_rr(XG, XS)                                                    \
        cgtgx3rr(W(XG), W(XG), W(XS))

#define cgtgx_ld(XG, MS, DS)                                                \
        cgtgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cgtgx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgn_rr(XG, XS)                                                    \
        cgtgn3rr(W(XG), W(XG), W(XS))

#define cgtgn_ld(XG, MS, DS)                                                \
        cgtgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cgtgn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegx_rr(XG, XS)                                                    \
        cgegx3rr(W(XG), W(XG), W(XS))

#define cgegx_ld(XG, MS, DS)                                                \
        cgegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegx3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cgegx3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegn_rr(XG, XS)                                                    \
        cgegn3rr(W(XG), W(XG), W(XS))

#define cgegn_ld(XG, MS, DS)                                                \
        cgegn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegn3rr(XD, XS, XT)                                                \
        EVW(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

#define cgegn3ld(XD, XS, MT, DT)                                            \
    ADR EVW(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1gx_ld(W(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_128    0xFF     /*  all satisfy the condition */

#define mk1hx_rx(RD)         /* not portable, do not use outside */         \
        VEX(RXB(RD),       0,    0x00, 0, 3, 1) EMITB(0x93)                 \
        MRM(REG(RD),    0x03,    0x01)

#define mkjgx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1gx_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1hx_rx(Reax)                                                      \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##16_128))                     \
        jeqxx_lb(lb)

/****************   packed byte-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgb_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgb_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgb_st(XS, MD, DD)                                                \
    ADR EVX(RXB(XS), RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgb_rr(XG, XS)                                                    \
        ck1gb_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(RXB(XG), RXB(XS),    0x00, 0, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvgb_ld(XG, MS, DS)                                                \
        ck1gb_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(RXB(XG), RXB(MS),    0x00, 0, 3, 1) EMITB(0x6F)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvgb_st(XS, MG, DG)                                                \
        ck1gb_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(RXB(XS), RXB(MG),    0x00, 0, 3, 1) EMITB(0x7F)                 \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgb_rr(XG, XS)                                                    \
        addgb3rr(W(XG), W(XG), W(XS))

#define addgb_ld(XG, MS, DS)                                                \
        addgb3ld(W(XG), W(XG), W(MS), W(DS))

#define addgb3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgb_rr(XG, XS)                                                    \
        adsgb3rr(W(XG), W(XG), W(XS))

#define adsgb_ld(XG, MS, DS)                                                \
        adsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgb3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgc_rr(XG, XS)                                                    \
        adsgc3rr(W(XG), W(XG), W(XS))

#define adsgc_ld(XG, MS, DS)                                                \
        adsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgc3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgb_rr(XG, XS)                                                    \
        subgb3rr(W(XG), W(XG), W(XS))

#define subgb_ld(XG, MS, DS)                                                \
        subgb3ld(W(XG), W(XG), W(MS), W(DS))

#define subgb3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgb_rr(XG, XS)                                                    \
        sbsgb3rr(W(XG), W(XG), W(XS))

#define sbsgb_ld(XG, MS, DS)                                                \
        sbsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgb3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgc_rr(XG, XS)                                                    \
        sbsgc3rr(W(XG), W(XG), W(XS))

#define sbsgc_ld(XG, MS, DS)                                                \
        sbsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgc3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgb_rr(XG, XS)                                                    \
        mulgb3rr(W(XG), W(XG), W(XS))

#define mulgb_ld(XG, MS, DS)                                                \
        mulgb3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgb3rr(XD, XS, XT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulgb_rx(W(XD))

#define mulgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
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
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgb_ri(XG, IS)                                                    \
        shlgb3ri(W(XG), W(XG), W(IS))

#define shlgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgb3ri(XD, XS, IT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

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
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

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
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgc3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

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
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgb_rx(W(XD))

#define svlgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
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
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgb3rr(W(XG), W(XG), W(XS))

#define svrgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgb3rr(XD, XS, XT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgb_rx(W(XD))

#define svrgb3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
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
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgc3rr(W(XG), W(XG), W(XS))

#define svrgc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgc3rr(XD, XS, XT)                                                \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgc_rx(W(XD))

#define svrgc3ld(XD, XS, MT, DT)                                            \
        movgb_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgb_ld(W(XD), W(MT), W(DT))                                       \
        movgb_st(W(XD), Mebp, inf_SCR02(0))                                 \
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
        movgb_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingb_rr(XG, XS)                                                    \
        mingb3rr(W(XG), W(XG), W(XS))

#define mingb_ld(XG, MS, DS)                                                \
        mingb3ld(W(XG), W(XG), W(MS), W(DS))

#define mingb3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingc_rr(XG, XS)                                                    \
        mingc3rr(W(XG), W(XG), W(XS))

#define mingc_ld(XG, MS, DS)                                                \
        mingc3ld(W(XG), W(XG), W(MS), W(DS))

#define mingc3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgb_rr(XG, XS)                                                    \
        maxgb3rr(W(XG), W(XG), W(XS))

#define maxgb_ld(XG, MS, DS)                                                \
        maxgb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgb3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgc_rr(XG, XS)                                                    \
        maxgc3rr(W(XG), W(XG), W(XS))

#define maxgc_ld(XG, MS, DS)                                                \
        maxgc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgc3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), 0, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), 0, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgb_rr(XG, XS)                                                    \
        ceqgb3rr(W(XG), W(XG), W(XS))

#define ceqgb_ld(XG, MS, DS)                                                \
        ceqgb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgb3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define ceqgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegb_rr(XG, XS)                                                    \
        cnegb3rr(W(XG), W(XG), W(XS))

#define cnegb_ld(XG, MS, DS)                                                \
        cnegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegb3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define cnegb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgb_rr(XG, XS)                                                    \
        cltgb3rr(W(XG), W(XG), W(XS))

#define cltgb_ld(XG, MS, DS)                                                \
        cltgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgb3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define cltgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgc_rr(XG, XS)                                                    \
        cltgc3rr(W(XG), W(XG), W(XS))

#define cltgc_ld(XG, MS, DS)                                                \
        cltgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgc3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define cltgc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegb_rr(XG, XS)                                                    \
        clegb3rr(W(XG), W(XG), W(XS))

#define clegb_ld(XG, MS, DS)                                                \
        clegb3ld(W(XG), W(XG), W(MS), W(DS))

#define clegb3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define clegb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegc_rr(XG, XS)                                                    \
        clegc3rr(W(XG), W(XG), W(XS))

#define clegc_ld(XG, MS, DS)                                                \
        clegc3ld(W(XG), W(XG), W(MS), W(DS))

#define clegc3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define clegc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgb_rr(XG, XS)                                                    \
        cgtgb3rr(W(XG), W(XG), W(XS))

#define cgtgb_ld(XG, MS, DS)                                                \
        cgtgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgb3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define cgtgb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgc_rr(XG, XS)                                                    \
        cgtgc3rr(W(XG), W(XG), W(XS))

#define cgtgc_ld(XG, MS, DS)                                                \
        cgtgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgc3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define cgtgc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegb_rr(XG, XS)                                                    \
        cgegb3rr(W(XG), W(XG), W(XS))

#define cgegb_ld(XG, MS, DS)                                                \
        cgegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegb3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define cgegb3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3E)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegc_rr(XG, XS)                                                    \
        cgegc3rr(W(XG), W(XG), W(XS))

#define cgegc_ld(XG, MS, DS)                                                \
        cgegc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegc3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

#define cgegc3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 0, 1, 3) EMITB(0x3F)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1gb_ld(W(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_128    0x0000   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_128    0xFFFF   /*  all satisfy the condition */

#define mk1bx_rx(RD)         /* not portable, do not use outside */         \
        VEW(RXB(RD),       0,    0x00, 0, 3, 1) EMITB(0x93)                 \
        MRM(REG(RD),    0x03,    0x01)

#define mkjgb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1gb_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1bx_rx(Reax)                                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##08_128))                     \
        jeqxx_lb(lb)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_128X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

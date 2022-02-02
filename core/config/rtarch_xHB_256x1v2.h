/******************************************************************************/
/* Copyright (c) 2013-2022 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_256X1V2_H
#define RT_RTARCH_XHB_256X1V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB_256x1v2.h: Implementation of x86_64 half+byte AVX1/2 instructions.
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

#if (RT_256X1 >= 1 && RT_256X1 <= 2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movax_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movax_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movax_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvax_rr(XG, XS)                                                    \
        andax_rr(W(XS), Xmm0)                                               \
        annax_rr(Xmm0, W(XG))                                               \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_rr(W(XG), Xmm0)

#define mmvax_ld(XG, MS, DS)                                                \
        notax_rx(Xmm0)                                                      \
        andax_rr(W(XG), Xmm0)                                               \
        annax_ld(Xmm0, W(MS), W(DS))                                        \
        orrax_rr(W(XG), Xmm0)

#define mmvax_st(XS, MG, DG)                                                \
        andax_rr(W(XS), Xmm0)                                               \
        annax_ld(Xmm0, W(MG), W(DG))                                        \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andax_rr(XG, XS)                                                    \
        andax3rr(W(XG), W(XG), W(XS))

#define andax_ld(XG, MS, DS)                                                \
        andax3ld(W(XG), W(XG), W(MS), W(DS))

#define andax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annax_rr(XG, XS)                                                    \
        annax3rr(W(XG), W(XG), W(XS))

#define annax_ld(XG, MS, DS)                                                \
        annax3ld(W(XG), W(XG), W(MS), W(DS))

#define annax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrax_rr(XG, XS)                                                    \
        orrax3rr(W(XG), W(XG), W(XS))

#define orrax_ld(XG, MS, DS)                                                \
        orrax3ld(W(XG), W(XG), W(MS), W(DS))

#define orrax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornax_rr(XG, XS)                                                    \
        notax_rx(W(XG))                                                     \
        orrax_rr(W(XG), W(XS))

#define ornax_ld(XG, MS, DS)                                                \
        notax_rx(W(XG))                                                     \
        orrax_ld(W(XG), W(MS), W(DS))

#define ornax3rr(XD, XS, XT)                                                \
        notax_rr(W(XD), W(XS))                                              \
        orrax_rr(W(XD), W(XT))

#define ornax3ld(XD, XS, MT, DT)                                            \
        notax_rr(W(XD), W(XS))                                              \
        orrax_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorax_rr(XG, XS)                                                    \
        xorax3rr(W(XG), W(XG), W(XS))

#define xorax_ld(XG, MS, DS)                                                \
        xorax3ld(W(XG), W(XG), W(MS), W(DS))

#define xorax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notax_rx(XG)                                                        \
        notax_rr(W(XG), W(XG))

#define notax_rr(XD, XS)                                                    \
        annax3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_256X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addax_rx(W(XD))

#define addax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addax_rx(W(XD))

#define addax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
        adsax3rr(W(XG), W(XG), W(XS))

#define adsax_ld(XG, MS, DS)                                                \
        adsax3ld(W(XG), W(XG), W(MS), W(DS))

#define adsax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsax_rx(W(XD))

#define adsax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsax_rx(W(XD))

#define adsax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
        adsan3rr(W(XG), W(XG), W(XS))

#define adsan_ld(XG, MS, DS)                                                \
        adsan3ld(W(XG), W(XG), W(MS), W(DS))

#define adsan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsan_rx(W(XD))

#define adsan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsan_rx(W(XD))

#define adsan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        subax_rx(W(XD))

#define subax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        subax_rx(W(XD))

#define subax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
        sbsax3rr(W(XG), W(XG), W(XS))

#define sbsax_ld(XG, MS, DS)                                                \
        sbsax3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsax_rx(W(XD))

#define sbsax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsax_rx(W(XD))

#define sbsax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
        sbsan3rr(W(XG), W(XG), W(XS))

#define sbsan_ld(XG, MS, DS)                                                \
        sbsan3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsan_rx(W(XD))

#define sbsan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsan_rx(W(XD))

#define sbsan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulax_rx(W(XD))

#define mulax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulax_rx(W(XD))

#define mulax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mulgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shlax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shrax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ri(W(XD), W(XS), W(IT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ri(W(XD), W(IT))                                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shran3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrgn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrgn_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addax_rr(XG, XS)                                                    \
        addax3rr(W(XG), W(XG), W(XS))

#define addax_ld(XG, MS, DS)                                                \
        addax3ld(W(XG), W(XG), W(MS), W(DS))

#define addax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xFD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsax_rr(XG, XS)                                                    \
        adsax3rr(W(XG), W(XG), W(XS))

#define adsax_ld(XG, MS, DS)                                                \
        adsax3ld(W(XG), W(XG), W(MS), W(DS))

#define adsax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDD)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsan_rr(XG, XS)                                                    \
        adsan3rr(W(XG), W(XG), W(XS))

#define adsan_ld(XG, MS, DS)                                                \
        adsan3ld(W(XG), W(XG), W(MS), W(DS))

#define adsan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xED)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subax_rr(XG, XS)                                                    \
        subax3rr(W(XG), W(XG), W(XS))

#define subax_ld(XG, MS, DS)                                                \
        subax3ld(W(XG), W(XG), W(MS), W(DS))

#define subax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsax_rr(XG, XS)                                                    \
        sbsax3rr(W(XG), W(XG), W(XS))

#define sbsax_ld(XG, MS, DS)                                                \
        sbsax3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsan_rr(XG, XS)                                                    \
        sbsan3rr(W(XG), W(XG), W(XS))

#define sbsan_ld(XG, MS, DS)                                                \
        sbsan3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xE9)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulax_rr(XG, XS)                                                    \
        mulax3rr(W(XG), W(XG), W(XS))

#define mulax_ld(XG, MS, DS)                                                \
        mulax3ld(W(XG), W(XG), W(MS), W(DS))

#define mulax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD5)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlax_ri(XG, IS)                                                    \
        shlax3ri(W(XG), W(XG), W(IS))

#define shlax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlax3ld(W(XG), W(XG), W(MS), W(DS))

#define shlax3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrax_ri(XG, IS)                                                    \
        shrax3ri(W(XG), W(XG), W(IS))

#define shrax_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrax3ld(W(XG), W(XG), W(MS), W(DS))

#define shrax3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shran_ri(XG, IS)                                                    \
        shran3ri(W(XG), W(XG), W(IS))

#define shran_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shran3ld(W(XG), W(XG), W(MS), W(DS))

#define shran3ri(XD, XS, IT)                                                \
        VEX(0,       RXB(XS), REN(XD), 1, 1, 1) EMITB(0x71)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shran3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xE1)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlax3rr(W(XG), W(XG), W(XS))

#define svlax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlax3ld(W(XG), W(XG), W(MS), W(DS))

#define svlax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlax_rx(W(XD))

#define svlax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlax_rx(W(XD))

#define svlax_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrax_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrax3rr(W(XG), W(XG), W(XS))

#define svrax_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrax3ld(W(XG), W(XG), W(MS), W(DS))

#define svrax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrax_rx(W(XD))

#define svrax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrax_rx(W(XD))

#define svrax_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svran_rr(XG, XS)     /* variable shift with per-elem count */       \
        svran3rr(W(XG), W(XG), W(XS))

#define svran_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svran3ld(W(XG), W(XG), W(MS), W(DS))

#define svran3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svran_rx(W(XD))

#define svran3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svran_rx(W(XD))

#define svran_rx(XD) /* not portable, do not use outside */                 \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed half-precision integer compare   ******************/

#if (RT_256X1 < 2)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minax_rr(XG, XS)                                                    \
        minax3rr(W(XG), W(XG), W(XS))

#define minax_ld(XG, MS, DS)                                                \
        minax3ld(W(XG), W(XG), W(MS), W(DS))

#define minax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minax_rx(W(XD))

#define minax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minax_rx(W(XD))

#define minax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minan_rr(XG, XS)                                                    \
        minan3rr(W(XG), W(XG), W(XS))

#define minan_ld(XG, MS, DS)                                                \
        minan3ld(W(XG), W(XG), W(MS), W(DS))

#define minan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minan_rx(W(XD))

#define minan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minan_rx(W(XD))

#define minan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxax_rr(XG, XS)                                                    \
        maxax3rr(W(XG), W(XG), W(XS))

#define maxax_ld(XG, MS, DS)                                                \
        maxax3ld(W(XG), W(XG), W(MS), W(DS))

#define maxax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxax_rx(W(XD))

#define maxax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxax_rx(W(XD))

#define maxax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxan_rr(XG, XS)                                                    \
        maxan3rr(W(XG), W(XG), W(XS))

#define maxan_ld(XG, MS, DS)                                                \
        maxan3ld(W(XG), W(XG), W(MS), W(DS))

#define maxan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxan_rx(W(XD))

#define maxan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxan_rx(W(XD))

#define maxan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqax_rr(XG, XS)                                                    \
        ceqax3rr(W(XG), W(XG), W(XS))

#define ceqax_ld(XG, MS, DS)                                                \
        ceqax3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqax3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        ceqax_rx(W(XD))

#define ceqax3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        ceqax_rx(W(XD))

#define ceqax_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        ceqgx_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtan_rr(XG, XS)                                                    \
        cgtan3rr(W(XG), W(XG), W(XS))

#define cgtan_ld(XG, MS, DS)                                                \
        cgtan3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtan3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        cgtan_rx(W(XD))

#define cgtan3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        cgtan_rx(W(XD))

#define cgtan_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        cgtgn_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minax_rr(XG, XS)                                                    \
        minax3rr(W(XG), W(XG), W(XS))

#define minax_ld(XG, MS, DS)                                                \
        minax3ld(W(XG), W(XG), W(MS), W(DS))

#define minax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3A)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minan_rr(XG, XS)                                                    \
        minan3rr(W(XG), W(XG), W(XS))

#define minan_ld(XG, MS, DS)                                                \
        minan3ld(W(XG), W(XG), W(MS), W(DS))

#define minan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxax_rr(XG, XS)                                                    \
        maxax3rr(W(XG), W(XG), W(XS))

#define maxax_ld(XG, MS, DS)                                                \
        maxax3ld(W(XG), W(XG), W(MS), W(DS))

#define maxax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxan_rr(XG, XS)                                                    \
        maxan3rr(W(XG), W(XG), W(XS))

#define maxan_ld(XG, MS, DS)                                                \
        maxan3ld(W(XG), W(XG), W(MS), W(DS))

#define maxan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqax_rr(XG, XS)                                                    \
        ceqax3rr(W(XG), W(XG), W(XS))

#define ceqax_ld(XG, MS, DS)                                                \
        ceqax3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqax3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqax3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x75)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtan_rr(XG, XS)                                                    \
        cgtan3rr(W(XG), W(XG), W(XS))

#define cgtan_ld(XG, MS, DS)                                                \
        cgtan3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtan3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtan3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x65)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneax_rr(XG, XS)                                                    \
        cneax3rr(W(XG), W(XG), W(XS))

#define cneax_ld(XG, MS, DS)                                                \
        cneax3ld(W(XG), W(XG), W(MS), W(DS))

#define cneax3rr(XD, XS, XT)                                                \
        ceqax3rr(W(XD), W(XS), W(XT))                                       \
        notax_rx(W(XD))

#define cneax3ld(XD, XS, MT, DT)                                            \
        ceqax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notax_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltax_rr(XG, XS)                                                    \
        cltax3rr(W(XG), W(XG), W(XS))

#define cltax_ld(XG, MS, DS)                                                \
        cltax3ld(W(XG), W(XG), W(MS), W(DS))

#define cltax3rr(XD, XS, XT)                                                \
        minax3rr(W(XD), W(XS), W(XT))                                       \
        cneax_rr(W(XD), W(XT))

#define cltax3ld(XD, XS, MT, DT)                                            \
        minax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneax_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltan_rr(XG, XS)                                                    \
        cltan3rr(W(XG), W(XG), W(XS))

#define cltan_ld(XG, MS, DS)                                                \
        cltan3ld(W(XG), W(XG), W(MS), W(DS))

#define cltan3rr(XD, XS, XT)                                                \
        cgtan3rr(W(XD), W(XT), W(XS))

#define cltan3ld(XD, XS, MT, DT)                                            \
        minan3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneax_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleax_rr(XG, XS)                                                    \
        cleax3rr(W(XG), W(XG), W(XS))

#define cleax_ld(XG, MS, DS)                                                \
        cleax3ld(W(XG), W(XG), W(MS), W(DS))

#define cleax3rr(XD, XS, XT)                                                \
        maxax3rr(W(XD), W(XS), W(XT))                                       \
        ceqax_rr(W(XD), W(XT))

#define cleax3ld(XD, XS, MT, DT)                                            \
        maxax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqax_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clean_rr(XG, XS)                                                    \
        clean3rr(W(XG), W(XG), W(XS))

#define clean_ld(XG, MS, DS)                                                \
        clean3ld(W(XG), W(XG), W(MS), W(DS))

#define clean3rr(XD, XS, XT)                                                \
        cgtan3rr(W(XD), W(XS), W(XT))                                       \
        notax_rx(W(XD))

#define clean3ld(XD, XS, MT, DT)                                            \
        cgtan3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notax_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtax_rr(XG, XS)                                                    \
        cgtax3rr(W(XG), W(XG), W(XS))

#define cgtax_ld(XG, MS, DS)                                                \
        cgtax3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtax3rr(XD, XS, XT)                                                \
        maxax3rr(W(XD), W(XS), W(XT))                                       \
        cneax_rr(W(XD), W(XT))

#define cgtax3ld(XD, XS, MT, DT)                                            \
        maxax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneax_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeax_rr(XG, XS)                                                    \
        cgeax3rr(W(XG), W(XG), W(XS))

#define cgeax_ld(XG, MS, DS)                                                \
        cgeax3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeax3rr(XD, XS, XT)                                                \
        minax3rr(W(XD), W(XS), W(XT))                                       \
        ceqax_rr(W(XD), W(XT))

#define cgeax3ld(XD, XS, MT, DT)                                            \
        minax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqax_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgean_rr(XG, XS)                                                    \
        cgean3rr(W(XG), W(XG), W(XS))

#define cgean_ld(XG, MS, DS)                                                \
        cgean3ld(W(XG), W(XG), W(MS), W(DS))

#define cgean3rr(XD, XS, XT)                                                \
        minan3rr(W(XD), W(XS), W(XT))                                       \
        ceqax_rr(W(XD), W(XT))

#define cgean3ld(XD, XS, MT, DT)                                            \
        minan3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqax_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_256    0xFF     /*  all satisfy the condition */

#define mkjax_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        VEX(0,       RXB(XS),    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        shlcx_ri(W(XS), IB(16))                                             \
        VEX(1,       RXB(XS),    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x07,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##16_256 & 0x1) << 1)))  \
        MRM(0x00,       0x03, 0x07)                                         \
        movax_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##16_256))                     \
        jeqxx_lb(lb)

/****************   packed byte-precision generic move/logic   ****************/

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvab_rr(XG, XS)                                                    \
        andax_rr(W(XS), Xmm0)                                               \
        annax_rr(Xmm0, W(XG))                                               \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_rr(W(XG), Xmm0)

#define mmvab_ld(XG, MS, DS)                                                \
        notax_rx(Xmm0)                                                      \
        andax_rr(W(XG), Xmm0)                                               \
        annax_ld(Xmm0, W(MS), W(DS))                                        \
        orrax_rr(W(XG), Xmm0)

#define mmvab_st(XS, MG, DG)                                                \
        andax_rr(W(XS), Xmm0)                                               \
        annax_ld(Xmm0, W(MG), W(DG))                                        \
        orrax_rr(Xmm0, W(XS))                                               \
        movax_st(Xmm0, W(MG), W(DG))

/* move/logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

#if (RT_256X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addab_rr(XG, XS)                                                    \
        addab3rr(W(XG), W(XG), W(XS))

#define addab_ld(XG, MS, DS)                                                \
        addab3ld(W(XG), W(XG), W(MS), W(DS))

#define addab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addab_rx(W(XD))

#define addab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addab_rx(W(XD))

#define addab_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsab_rr(XG, XS)                                                    \
        adsab3rr(W(XG), W(XG), W(XS))

#define adsab_ld(XG, MS, DS)                                                \
        adsab3ld(W(XG), W(XG), W(MS), W(DS))

#define adsab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsab_rx(W(XD))

#define adsab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsab_rx(W(XD))

#define adsab_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsac_rr(XG, XS)                                                    \
        adsac3rr(W(XG), W(XG), W(XS))

#define adsac_ld(XG, MS, DS)                                                \
        adsac3ld(W(XG), W(XG), W(MS), W(DS))

#define adsac3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsac_rx(W(XD))

#define adsac3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsac_rx(W(XD))

#define adsac_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        adsgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subab_rr(XG, XS)                                                    \
        subab3rr(W(XG), W(XG), W(XS))

#define subab_ld(XG, MS, DS)                                                \
        subab3ld(W(XG), W(XG), W(MS), W(DS))

#define subab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        subab_rx(W(XD))

#define subab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        subab_rx(W(XD))

#define subab_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsab_rr(XG, XS)                                                    \
        sbsab3rr(W(XG), W(XG), W(XS))

#define sbsab_ld(XG, MS, DS)                                                \
        sbsab3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsab_rx(W(XD))

#define sbsab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsab_rx(W(XD))

#define sbsab_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsac_rr(XG, XS)                                                    \
        sbsac3rr(W(XG), W(XG), W(XS))

#define sbsac_ld(XG, MS, DS)                                                \
        sbsac3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsac3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsac_rx(W(XD))

#define sbsac3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsac_rx(W(XD))

#define sbsac_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        sbsgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addab_rr(XG, XS)                                                    \
        addab3rr(W(XG), W(XG), W(XS))

#define addab_ld(XG, MS, DS)                                                \
        addab3ld(W(XG), W(XG), W(MS), W(DS))

#define addab3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addab3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xFC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsab_rr(XG, XS)                                                    \
        adsab3rr(W(XG), W(XG), W(XS))

#define adsab_ld(XG, MS, DS)                                                \
        adsab3ld(W(XG), W(XG), W(MS), W(DS))

#define adsab3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsab3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsac_rr(XG, XS)                                                    \
        adsac3rr(W(XG), W(XG), W(XS))

#define adsac_ld(XG, MS, DS)                                                \
        adsac3ld(W(XG), W(XG), W(MS), W(DS))

#define adsac3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsac3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xEC)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subab_rr(XG, XS)                                                    \
        subab3rr(W(XG), W(XG), W(XS))

#define subab_ld(XG, MS, DS)                                                \
        subab3ld(W(XG), W(XG), W(MS), W(DS))

#define subab3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subab3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xF8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsab_rr(XG, XS)                                                    \
        sbsab3rr(W(XG), W(XG), W(XS))

#define sbsab_ld(XG, MS, DS)                                                \
        sbsab3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsab3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsab3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xD8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsac_rr(XG, XS)                                                    \
        sbsac3rr(W(XG), W(XG), W(XS))

#define sbsac_ld(XG, MS, DS)                                                \
        sbsac3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsac3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsac3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xE8)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulab_rr(XG, XS)                                                    \
        mulab3rr(W(XG), W(XG), W(XS))

#define mulab_ld(XG, MS, DS)                                                \
        mulab3ld(W(XG), W(XG), W(MS), W(DS))

#define mulab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulab_rx(W(XD))

#define mulab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulab_rx(W(XD))

#define mulab_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x11))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x11))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x12))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x12))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x13))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x13))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x14))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x14))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x15))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x15))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x16))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x16))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x17))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x17))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x19))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x19))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1F))                              \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlab_ri(XG, IS)                                                    \
        shlab3ri(W(XG), W(XG), W(IS))

#define shlab_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlab3ld(W(XG), W(XG), W(MS), W(DS))

#define shlab3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shlab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shlab_xx() /* not portable, do not use outside */                   \
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
        shlbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrab_ri(XG, IS)                                                    \
        shrab3ri(W(XG), W(XG), W(IS))

#define shrab_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrab3ld(W(XG), W(XG), W(MS), W(DS))

#define shrab3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shrab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrab_xx()                                                          \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shrab_xx() /* not portable, do not use outside */                   \
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
        shrbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrac_ri(XG, IS)                                                    \
        shrac3ri(W(XG), W(XG), W(IS))

#define shrac_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrac3ld(W(XG), W(XG), W(MS), W(DS))

#define shrac3ri(XD, XS, IT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrac_xx()                                                          \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shrac3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrac_xx()                                                          \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#define shrac_xx() /* not portable, do not use outside */                   \
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
        shrbn_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x11))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x12))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x13))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x15))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x16))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x17))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x19))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1F))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlab_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlab3rr(W(XG), W(XG), W(XS))

#define svlab_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlab3ld(W(XG), W(XG), W(MS), W(DS))

#define svlab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlab_rx(W(XD))

#define svlab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlab_rx(W(XD))

#define svlab_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrab_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrab3rr(W(XG), W(XG), W(XS))

#define svrab_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrab3ld(W(XG), W(XG), W(MS), W(DS))

#define svrab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrab_rx(W(XD))

#define svrab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrab_rx(W(XD))

#define svrab_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrac_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrac3rr(W(XG), W(XG), W(XS))

#define svrac_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrac3ld(W(XG), W(XG), W(MS), W(DS))

#define svrac3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrac_rx(W(XD))

#define svrac3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrac_rx(W(XD))

#define svrac_rx(XD) /* not portable, do not use outside */                 \
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
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1F))                                    \
        stack_ld(Recx)                                                      \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

#if (RT_256X1 < 2)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minab_rr(XG, XS)                                                    \
        minab3rr(W(XG), W(XG), W(XS))

#define minab_ld(XG, MS, DS)                                                \
        minab3ld(W(XG), W(XG), W(MS), W(DS))

#define minab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minab_rx(W(XD))

#define minab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minab_rx(W(XD))

#define minab_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minac_rr(XG, XS)                                                    \
        minac3rr(W(XG), W(XG), W(XS))

#define minac_ld(XG, MS, DS)                                                \
        minac3ld(W(XG), W(XG), W(MS), W(DS))

#define minac3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minac_rx(W(XD))

#define minac3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minac_rx(W(XD))

#define minac_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mingc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mingc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxab_rr(XG, XS)                                                    \
        maxab3rr(W(XG), W(XG), W(XS))

#define maxab_ld(XG, MS, DS)                                                \
        maxab3ld(W(XG), W(XG), W(MS), W(DS))

#define maxab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxab_rx(W(XD))

#define maxab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxab_rx(W(XD))

#define maxab_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxac_rr(XG, XS)                                                    \
        maxac3rr(W(XG), W(XG), W(XS))

#define maxac_ld(XG, MS, DS)                                                \
        maxac3ld(W(XG), W(XG), W(MS), W(DS))

#define maxac3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxac_rx(W(XD))

#define maxac3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxac_rx(W(XD))

#define maxac_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqab_rr(XG, XS)                                                    \
        ceqab3rr(W(XG), W(XG), W(XS))

#define ceqab_ld(XG, MS, DS)                                                \
        ceqab3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqab3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        ceqab_rx(W(XD))

#define ceqab3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        ceqab_rx(W(XD))

#define ceqab_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqgb_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        ceqgb_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtac_rr(XG, XS)                                                    \
        cgtac3rr(W(XG), W(XG), W(XS))

#define cgtac_ld(XG, MS, DS)                                                \
        cgtac3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtac3rr(XD, XS, XT)                                                \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_st(W(XT), Mebp, inf_SCR02(0))                                 \
        cgtac_rx(W(XD))

#define cgtac3ld(XD, XS, MT, DT)                                            \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR02(0))                                 \
        cgtac_rx(W(XD))

#define cgtac_rx(XD) /* not portable, do not use outside */                 \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtgc_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movgx_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        cgtgc_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movgx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X1 >= 2, AVX2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minab_rr(XG, XS)                                                    \
        minab3rr(W(XG), W(XG), W(XS))

#define minab_ld(XG, MS, DS)                                                \
        minab3ld(W(XG), W(XG), W(MS), W(DS))

#define minab3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minab3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDA)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minac_rr(XG, XS)                                                    \
        minac3rr(W(XG), W(XG), W(XS))

#define minac_ld(XG, MS, DS)                                                \
        minac3ld(W(XG), W(XG), W(MS), W(DS))

#define minac3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minac3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x38)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxab_rr(XG, XS)                                                    \
        maxab3rr(W(XG), W(XG), W(XS))

#define maxab_ld(XG, MS, DS)                                                \
        maxab3ld(W(XG), W(XG), W(MS), W(DS))

#define maxab3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxab3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0xDE)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxac_rr(XG, XS)                                                    \
        maxac3rr(W(XG), W(XG), W(XS))

#define maxac_ld(XG, MS, DS)                                                \
        maxac3ld(W(XG), W(XG), W(MS), W(DS))

#define maxac3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxac3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 2) EMITB(0x3C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqab_rr(XG, XS)                                                    \
        ceqab3rr(W(XG), W(XG), W(XS))

#define ceqab_ld(XG, MS, DS)                                                \
        ceqab3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqab3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqab3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x74)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtac_rr(XG, XS)                                                    \
        cgtac3rr(W(XG), W(XG), W(XS))

#define cgtac_ld(XG, MS, DS)                                                \
        cgtac3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtac3rr(XD, XS, XT)                                                \
        VEX(RXB(XD), RXB(XT), REN(XS), 1, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtac3ld(XD, XS, MT, DT)                                            \
    ADR VEX(RXB(XD), RXB(MT), REN(XS), 1, 1, 1) EMITB(0x64)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256X1 >= 2, AVX2 */

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneab_rr(XG, XS)                                                    \
        cneab3rr(W(XG), W(XG), W(XS))

#define cneab_ld(XG, MS, DS)                                                \
        cneab3ld(W(XG), W(XG), W(MS), W(DS))

#define cneab3rr(XD, XS, XT)                                                \
        ceqab3rr(W(XD), W(XS), W(XT))                                       \
        notax_rx(W(XD))

#define cneab3ld(XD, XS, MT, DT)                                            \
        ceqab3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notax_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltab_rr(XG, XS)                                                    \
        cltab3rr(W(XG), W(XG), W(XS))

#define cltab_ld(XG, MS, DS)                                                \
        cltab3ld(W(XG), W(XG), W(MS), W(DS))

#define cltab3rr(XD, XS, XT)                                                \
        minab3rr(W(XD), W(XS), W(XT))                                       \
        cneab_rr(W(XD), W(XT))

#define cltab3ld(XD, XS, MT, DT)                                            \
        minab3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneab_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltac_rr(XG, XS)                                                    \
        cltac3rr(W(XG), W(XG), W(XS))

#define cltac_ld(XG, MS, DS)                                                \
        cltac3ld(W(XG), W(XG), W(MS), W(DS))

#define cltac3rr(XD, XS, XT)                                                \
        cgtac3rr(W(XD), W(XT), W(XS))

#define cltac3ld(XD, XS, MT, DT)                                            \
        minac3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneab_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleab_rr(XG, XS)                                                    \
        cleab3rr(W(XG), W(XG), W(XS))

#define cleab_ld(XG, MS, DS)                                                \
        cleab3ld(W(XG), W(XG), W(MS), W(DS))

#define cleab3rr(XD, XS, XT)                                                \
        maxab3rr(W(XD), W(XS), W(XT))                                       \
        ceqab_rr(W(XD), W(XT))

#define cleab3ld(XD, XS, MT, DT)                                            \
        maxab3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqab_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleac_rr(XG, XS)                                                    \
        cleac3rr(W(XG), W(XG), W(XS))

#define cleac_ld(XG, MS, DS)                                                \
        cleac3ld(W(XG), W(XG), W(MS), W(DS))

#define cleac3rr(XD, XS, XT)                                                \
        cgtac3rr(W(XD), W(XS), W(XT))                                       \
        notax_rx(W(XD))

#define cleac3ld(XD, XS, MT, DT)                                            \
        cgtac3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notax_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtab_rr(XG, XS)                                                    \
        cgtab3rr(W(XG), W(XG), W(XS))

#define cgtab_ld(XG, MS, DS)                                                \
        cgtab3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtab3rr(XD, XS, XT)                                                \
        maxab3rr(W(XD), W(XS), W(XT))                                       \
        cneab_rr(W(XD), W(XT))

#define cgtab3ld(XD, XS, MT, DT)                                            \
        maxab3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneab_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeab_rr(XG, XS)                                                    \
        cgeab3rr(W(XG), W(XG), W(XS))

#define cgeab_ld(XG, MS, DS)                                                \
        cgeab3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeab3rr(XD, XS, XT)                                                \
        minab3rr(W(XD), W(XS), W(XT))                                       \
        ceqab_rr(W(XD), W(XT))

#define cgeab3ld(XD, XS, MT, DT)                                            \
        minab3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqab_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeac_rr(XG, XS)                                                    \
        cgeac3rr(W(XG), W(XG), W(XS))

#define cgeac_ld(XG, MS, DS)                                                \
        cgeac3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeac3rr(XD, XS, XT)                                                \
        minac3rr(W(XD), W(XS), W(XT))                                       \
        ceqab_rr(W(XD), W(XT))

#define cgeac3ld(XD, XS, MT, DT)                                            \
        minac3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqab_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_256    0xFF     /*  all satisfy the condition */

#define bsncx_rx(XS, mask) /* not portable, do not use outside */           \
        VEX(1,       RXB(XS),    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x07,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##08_256 & 0x1) << 1)))  \
        MRM(0x00,       0x03, 0x07)

#define mkjab_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movax_st(W(XS), Mebp, inf_SCR01(0))                                 \
        VEX(0,       RXB(XS),    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        shlcx_ri(W(XS), IB(8))                                              \
        bsncx_rx(W(XS), mask)                                               \
        shlcx_ri(W(XS), IB(8))                                              \
        bsncx_rx(W(XS), mask)                                               \
        shlcx_ri(W(XS), IB(8))                                              \
        bsncx_rx(W(XS), mask)                                               \
        movax_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##08_256))                     \
        jeqxx_lb(lb)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHB_256X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHF_256X1V8_H
#define RT_RTARCH_XHF_256X1V8_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHF_256x1v8.h: Implementation of x86_64 AVX512VL/fp16 instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * and contains architecture-specific extensions
 * outside of the common assembler core.
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

#if (RT_256X1 == 8)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmax_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmns_st(W(XS), W(MD), W(DD))

/*************   packed half-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negas_rx(XG)                                                        \
        negas_rr(W(XG), W(XG))

#define negas_rr(XD, XS)                                                    \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), Mebp, inf_GPC06_32)                                 \
        shrcx_ri(W(XD), IB(16))                                             \
        xorcx_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        xorcx_ld(W(XD), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addas_rr(XG, XS)                                                    \
        addas3rr(W(XG), W(XG), W(XS))

#define addas_ld(XG, MS, DS)                                                \
        addas3ld(W(XG), W(XG), W(MS), W(DS))

#define addas3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addas3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subas_rr(XG, XS)                                                    \
        subas3rr(W(XG), W(XG), W(XS))

#define subas_ld(XG, MS, DS)                                                \
        subas3ld(W(XG), W(XG), W(MS), W(DS))

#define subas3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subas3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulas_rr(XG, XS)                                                    \
        mulas3rr(W(XG), W(XG), W(XS))

#define mulas_ld(XG, MS, DS)                                                \
        mulas3ld(W(XG), W(XG), W(MS), W(DS))

#define mulas3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulas3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divas_rr(XG, XS)                                                    \
        divas3rr(W(XG), W(XG), W(XS))

#define divas_ld(XG, MS, DS)                                                \
        divas3ld(W(XG), W(XG), W(MS), W(DS))

#define divas3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divas3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqras_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqras_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rceas_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsas_rr(XG, XS) /* destroys XS */                                  \
        mulas_rr(W(XS), W(XG))                                              \
        mulas_rr(W(XS), W(XG))                                              \
        addas_rr(W(XG), W(XG))                                              \
        subas_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rseas_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 1, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmaas_rr(XG, XS, XT)                                                \
    ADR EFX(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaas_ld(XG, XS, MT, DT)                                            \
    ADR EFX(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsas_rr(XG, XS, XT)                                                \
    ADR EFX(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsas_ld(XG, XS, MT, DT)                                            \
    ADR EFX(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minas_rr(XG, XS)                                                    \
        minas3rr(W(XG), W(XG), W(XS))

#define minas_ld(XG, MS, DS)                                                \
        minas3ld(W(XG), W(XG), W(MS), W(DS))

#define minas3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minas3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxas_rr(XG, XS)                                                    \
        maxas3rr(W(XG), W(XG), W(XS))

#define maxas_ld(XG, MS, DS)                                                \
        maxas3ld(W(XG), W(XG), W(MS), W(DS))

#define maxas3rr(XD, XS, XT)                                                \
        EFX(RXB(XD), RXB(XT), REN(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxas3ld(XD, XS, MT, DT)                                            \
    ADR EFX(RXB(XD), RXB(MT), REN(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqas_rr(XG, XS)                                                    \
        ceqas3rr(W(XG), W(XG), W(XS))

#define ceqas_ld(XG, MS, DS)                                                \
        ceqas3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqas3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

#define ceqas3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneas_rr(XG, XS)                                                    \
        cneas3rr(W(XG), W(XG), W(XS))

#define cneas_ld(XG, MS, DS)                                                \
        cneas3ld(W(XG), W(XG), W(MS), W(DS))

#define cneas3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

#define cneas3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltas_rr(XG, XS)                                                    \
        cltas3rr(W(XG), W(XG), W(XS))

#define cltas_ld(XG, MS, DS)                                                \
        cltas3ld(W(XG), W(XG), W(MS), W(DS))

#define cltas3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

#define cltas3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleas_rr(XG, XS)                                                    \
        cleas3rr(W(XG), W(XG), W(XS))

#define cleas_ld(XG, MS, DS)                                                \
        cleas3ld(W(XG), W(XG), W(MS), W(DS))

#define cleas3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

#define cleas3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtas_rr(XG, XS)                                                    \
        cgtas3rr(W(XG), W(XG), W(XS))

#define cgtas_ld(XG, MS, DS)                                                \
        cgtas3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtas3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

#define cgtas3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeas_rr(XG, XS)                                                    \
        cgeas3rr(W(XG), W(XG), W(XS))

#define cgeas_ld(XG, MS, DS)                                                \
        cgeas3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeas3rr(XD, XS, XT)                                                \
        EVX(0,       RXB(XT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

#define cgeas3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REN(XS), 1, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1ax_ld(W(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

    /* mkj for half-precision is defined in corresponding HB_256 header */

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzas_rr(XD, XS)     /* round towards zero */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzas_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzas_rr(XD, XS)     /* round towards zero */                       \
        EFX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzas_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpas_rr(XD, XS)     /* round towards +inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpas_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpas_rr(XD, XS)     /* round towards +inf */                       \
        EGX(RXB(XD), RXB(XS),    0x00, 2, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpas_ld(XD, MS, DS) /* round towards +inf */                       \
        movax_ld(W(XD), W(MS), W(DS))                                       \
        cvpas_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmas_rr(XD, XS)     /* round towards -inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmas_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmas_rr(XD, XS)     /* round towards -inf */                       \
        EGX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmas_ld(XD, MS, DS) /* round towards -inf */                       \
        movax_ld(W(XD), W(MS), W(DS))                                       \
        cvmas_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnnas_rr(XD, XS)     /* round towards near */                       \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnas_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvnas_rr(XD, XS)     /* round towards near */                       \
        cvtas_rr(W(XD), W(XS))

#define cvnas_ld(XD, MS, DS) /* round towards near */                       \
        cvtas_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnan_rr(XD, XS)     /* round towards near */                       \
        cvtan_rr(W(XD), W(XS))

#define cvnan_ld(XD, MS, DS) /* round towards near */                       \
        cvtan_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndas_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndas_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtas_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtas_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtan_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 1, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtan_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 1, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnax_rr(XD, XS)     /* round towards near */                       \
        cvtax_rr(W(XD), W(XS))

#define cvnax_ld(XD, MS, DS) /* round towards near */                       \
        cvtax_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtax_rr(XD, XS)                                                    \
        EFX(RXB(XD), RXB(XS),    0x00, 1, 3, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtax_ld(XD, MS, DS)                                                \
    ADR EFX(RXB(XD), RXB(MS),    0x00, 1, 3, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnras_rr(XD, XS, mode)                                              \
        EVX(RXB(XD), RXB(XS),    0x00, 1, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvras_rr(XD, XS, mode)                                              \
        EGX(RXB(XD), RXB(XS), 0x00, RT_SIMD_MODE_##mode&3, 2, 1) EMITB(0x7D)\
        MRM(REG(XD), MOD(XS), REG(XS))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHF_256X1V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

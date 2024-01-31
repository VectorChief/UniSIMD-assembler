/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHF_512X4V2_H
#define RT_RTARCH_XHF_512X4V2_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHF_512x1v8.h: Implementation of x86_64 AVX512F/fp16 instructions.
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

#if (RT_512X4 == 2)

#undef  K
#define K 2

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmmx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmns_st(W(XS), W(MD), W(DD))

/*************   packed half-precision floating-point arithmetic   ************/

/* neg (G = -G), (D = -S) */

#define negms_rx(XG)                                                        \
        negms_rr(W(XG), W(XG))

#define negms_rr(XD, XS)                                                    \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), Mebp, inf_GPC06_32)                                 \
        shrox_ri(W(XD), IB(16))                                             \
        xorox_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        xorox_ld(W(XD), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addms_rr(XG, XS)                                                    \
        addms3rr(W(XG), W(XG), W(XS))

#define addms_ld(XG, MS, DS)                                                \
        addms3ld(W(XG), W(XG), W(MS), W(DS))

#define addms3rr(XD, XS, XT)                                                \
        EFX(0,             0, REG(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(1,             1, REH(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(2,             2, REI(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(3,             3, REJ(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addms3ld(XD, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subms_rr(XG, XS)                                                    \
        subms3rr(W(XG), W(XG), W(XS))

#define subms_ld(XG, MS, DS)                                                \
        subms3ld(W(XG), W(XG), W(MS), W(DS))

#define subms3rr(XD, XS, XT)                                                \
        EFX(0,             0, REG(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(1,             1, REH(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(2,             2, REI(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subms3ld(XD, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulms_rr(XG, XS)                                                    \
        mulms3rr(W(XG), W(XG), W(XS))

#define mulms_ld(XG, MS, DS)                                                \
        mulms3ld(W(XG), W(XG), W(MS), W(DS))

#define mulms3rr(XD, XS, XT)                                                \
        EFX(0,             0, REG(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(1,             1, REH(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(2,             2, REI(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(3,             3, REJ(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulms3ld(XD, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* div (G = G / S), (D = S / T) if (#D != #T) */

#define divms_rr(XG, XS)                                                    \
        divms3rr(W(XG), W(XG), W(XS))

#define divms_ld(XG, MS, DS)                                                \
        divms3ld(W(XG), W(XG), W(MS), W(DS))

#define divms3rr(XD, XS, XT)                                                \
        EFX(0,             0, REG(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(1,             1, REH(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(2,             2, REI(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divms3ld(XD, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* sqr (D = sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define sqrms_rr(XD, XS)                                                    \
        EFX(0,             0,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(1,             1,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(2,             2,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(3,             3,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrms_ld(XD, MS, DS)                                                \
    ADR EFX(0,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EFX(1,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EFX(2,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EFX(3,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rcems_rr(XD, XS)                                                    \
        EFX(0,             0,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(1,             1,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(2,             2,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(3,             3,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsms_rr(XG, XS) /* destroys XS */                                  \
        mulms_rr(W(XS), W(XG))                                              \
        mulms_rr(W(XS), W(XG))                                              \
        addms_rr(W(XG), W(XG))                                              \
        subms_rr(W(XG), W(XS))

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#define rsems_rr(XD, XS)                                                    \
        EFX(0,             0,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(1,             1,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(2,             2,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(3,             3,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

/* fma (G = G + S * T) if (#G != #S && #G != #T) */

#define fmams_rr(XG, XS, XT)                                                \
    ADR EFX(0,             0, REG(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EFX(1,             1, REH(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EFX(2,             2, REI(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EFX(3,             3, REJ(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmams_ld(XG, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* fms (G = G - S * T) if (#G != #S && #G != #T) */

#define fmsms_rr(XG, XS, XT)                                                \
    ADR EFX(0,             0, REG(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EFX(1,             1, REH(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EFX(2,             2, REI(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EFX(3,             3, REJ(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsms_ld(XG, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/**************   packed half-precision floating-point compare   **************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minms_rr(XG, XS)                                                    \
        minms3rr(W(XG), W(XG), W(XS))

#define minms_ld(XG, MS, DS)                                                \
        minms3ld(W(XG), W(XG), W(MS), W(DS))

#define minms3rr(XD, XS, XT)                                                \
        EFX(0,             0, REG(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(1,             1, REH(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(2,             2, REI(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minms3ld(XD, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxms_rr(XG, XS)                                                    \
        maxms3rr(W(XG), W(XG), W(XS))

#define maxms_ld(XG, MS, DS)                                                \
        maxms3ld(W(XG), W(XG), W(MS), W(DS))

#define maxms3rr(XD, XS, XT)                                                \
        EFX(0,             0, REG(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(1,             1, REH(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(2,             2, REI(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EFX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxms3ld(XD, XS, MT, DT)                                            \
    ADR EFX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EFX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EFX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EFX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqms_rr(XG, XS)                                                    \
        ceqms3rr(W(XG), W(XG), W(XS))

#define ceqms_ld(XG, MS, DS)                                                \
        ceqms3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqms3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define ceqms3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x00))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x00))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x00))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x00))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnems_rr(XG, XS)                                                    \
        cnems3rr(W(XG), W(XG), W(XS))

#define cnems_ld(XG, MS, DS)                                                \
        cnems3ld(W(XG), W(XG), W(MS), W(DS))

#define cnems3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cnems3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x04))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x04))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x04))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x04))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltms_rr(XG, XS)                                                    \
        cltms3rr(W(XG), W(XG), W(XS))

#define cltms_ld(XG, MS, DS)                                                \
        cltms3ld(W(XG), W(XG), W(MS), W(DS))

#define cltms3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cltms3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x01))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x01))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x01))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x01))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clems_rr(XG, XS)                                                    \
        clems3rr(W(XG), W(XG), W(XS))

#define clems_ld(XG, MS, DS)                                                \
        clems3ld(W(XG), W(XG), W(MS), W(DS))

#define clems3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define clems3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x02))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x02))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x02))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x02))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtms_rr(XG, XS)                                                    \
        cgtms3rr(W(XG), W(XG), W(XS))

#define cgtms_ld(XG, MS, DS)                                                \
        cgtms3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtms3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cgtms3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x06))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x06))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x06))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x06))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgems_rr(XG, XS)                                                    \
        cgems3rr(W(XG), W(XG), W(XS))

#define cgems_ld(XG, MS, DS)                                                \
        cgems3ld(W(XG), W(XG), W(MS), W(DS))

#define cgems3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

#define cgems3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x05))                           \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x05))                           \
        mz1mx_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x05))                           \
        mz1mx_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 3) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x05))                           \
        mz1mx_ld(Z(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

    /* mkj for half-precision is defined in corresponding HB_512 header */

/**************   packed half-precision floating-point convert   **************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnzms_rr(XD, XS)     /* round towards zero */                       \
        EVX(0,             0,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVX(1,             1,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVX(2,             2,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVX(3,             3,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzms_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x03))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x03))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x03))

#define cvzms_rr(XD, XS)     /* round towards zero */                       \
        EFX(0,             0,    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(1,             1,    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(2,             2,    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(3,             3,    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzms_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EFX(0,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EFX(1,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EFX(2,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EFX(3,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7C)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnpms_rr(XD, XS)     /* round towards +inf */                       \
        EVX(0,             0,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVX(1,             1,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVX(2,             2,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVX(3,             3,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpms_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x02))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x02))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x02))

#define cvpms_rr(XD, XS)     /* round towards +inf */                       \
        EGX(0,             0,    0x00, 2, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(1,             1,    0x00, 2, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(2,             2,    0x00, 2, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(3,             3,    0x00, 2, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpms_ld(XD, MS, DS) /* round towards +inf */                       \
        movmx_ld(W(XD), W(MS), W(DS))                                       \
        cvpms_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnmms_rr(XD, XS)     /* round towards -inf */                       \
        EVX(0,             0,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVX(1,             1,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVX(2,             2,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVX(3,             3,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmms_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x01))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x01))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x01))

#define cvmms_rr(XD, XS)     /* round towards -inf */                       \
        EGX(0,             0,    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(1,             1,    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(2,             2,    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(3,             3,    0x00, 1, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmms_ld(XD, MS, DS) /* round towards -inf */                       \
        movmx_ld(W(XD), W(MS), W(DS))                                       \
        cvmms_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnnms_rr(XD, XS)     /* round towards near */                       \
        EVX(0,             0,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVX(1,             1,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVX(2,             2,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVX(3,             3,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnms_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x00))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x00))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x00))

#define cvnms_rr(XD, XS)     /* round towards near */                       \
        cvtms_rr(W(XD), W(XS))

#define cvnms_ld(XD, MS, DS) /* round towards near */                       \
        cvtms_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnmn_rr(XD, XS)     /* round towards near */                       \
        cvtmn_rr(W(XD), W(XS))

#define cvnmn_ld(XD, MS, DS) /* round towards near */                       \
        cvtmn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define rndms_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVX(1,             1,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVX(2,             2,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVX(3,             3,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndms_ld(XD, MS, DS)                                                \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x04))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x04))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x04))

#define cvtms_rr(XD, XS)                                                    \
        EFX(0,             0,    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(1,             1,    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(2,             2,    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(3,             3,    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtms_ld(XD, MS, DS)                                                \
    ADR EFX(0,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EFX(1,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EFX(2,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EFX(3,       RXB(MS),    0x00, K, 1, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from control register (set in FCTRL blocks) */

#define cvtmn_rr(XD, XS)                                                    \
        EFX(0,             0,    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(1,             1,    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(2,             2,    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EFX(3,             3,    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtmn_ld(XD, MS, DS)                                                \
    ADR EFX(0,       RXB(MS),    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EFX(1,       RXB(MS),    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EFX(2,       RXB(MS),    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EFX(3,       RXB(MS),    0x00, K, 2, 1) EMITB(0x7D)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks) */

#define rnrms_rr(XD, XS, mode)                                              \
        EVX(0,             0,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVX(1,             1,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVX(2,             2,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVX(3,             3,    0x00, K, 0, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrms_rr(XD, XS, mode)                                              \
        EGX(0,             0, 0x00, RT_SIMD_MODE_##mode&3, 2, 1) EMITB(0x7D)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(1,             1, 0x00, RT_SIMD_MODE_##mode&3, 2, 1) EMITB(0x7D)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(2,             2, 0x00, RT_SIMD_MODE_##mode&3, 2, 1) EMITB(0x7D)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EGX(3,             3, 0x00, RT_SIMD_MODE_##mode&3, 2, 1) EMITB(0x7D)\
        MRM(REG(XD), MOD(XS), REG(XS))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_512X4 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_XHF_512X4V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

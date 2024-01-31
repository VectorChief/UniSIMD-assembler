/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_512X4V2_H
#define RT_RTARCH_X32_512X4V2_H

#include "rtarch_x64.h"

#define RT_SIMD_REGS            8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_512x4v2.h: Implementation of x86_64 fp32 AVX512F/DQ quaded ops.
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
 * cmdi*_** - applies [cmd] to 32-bit elements SIMD args, packed-128-bit
 * cmdj*_** - applies [cmd] to 64-bit elements SIMD args, packed-128-bit
 * cmdl*_** - applies [cmd] to L-size elements SIMD args, packed-128-bit
 *
 * cmdc*_** - applies [cmd] to 32-bit elements SIMD args, packed-256-bit
 * cmdd*_** - applies [cmd] to 64-bit elements SIMD args, packed-256-bit
 * cmdf*_** - applies [cmd] to L-size elements SIMD args, packed-256-bit
 *
 * cmdo*_** - applies [cmd] to 32-bit elements SIMD args, packed-var-len
 * cmdp*_** - applies [cmd] to L-size elements SIMD args, packed-var-len
 * cmdq*_** - applies [cmd] to 64-bit elements SIMD args, packed-var-len
 *
 * cmdr*_** - applies [cmd] to 32-bit elements ELEM args, scalar-fp-only
 * cmds*_** - applies [cmd] to L-size elements ELEM args, scalar-fp-only
 * cmdt*_** - applies [cmd] to 64-bit elements ELEM args, scalar-fp-only
 *
 * cmd*x_** - applies [cmd] to SIMD/BASE unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to SIMD/BASE   signed integer args, [n] - negatable
 * cmd*s_** - applies [cmd] to SIMD/ELEM floating point   args, [s] - scalable
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

#if (RT_512X4 >= 1 && RT_512X4 <= 2)

#if (RT_512X4 == 1)

#ifndef RT_RTARCH_X64_256X1V2_H
#undef  RT_256X1
#define RT_256X1  2
#include "rtarch_x64_256x1v2.h"
#endif /* RT_RTARCH_X64_256X1V2_H */

#define ck1ox_rm(XS, MT, DT) /* not portable, do not use outside */         \
    ADR EVX(0,       RXB(MT), REN(XS), K, 1, 1) EMITB(0x76)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

#define mz1ox_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR EZX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#else  /* (RT_512X4 == 2) */

#ifndef RT_RTARCH_X64_256X1V8_H
#undef  RT_256X1
#define RT_256X1  8
#include "rtarch_x64_256x1v8.h"
#endif /* RT_RTARCH_X64_256X1V8_H */

#define ck1ox_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(0,       RXB(XS),    0x00, K, 2, 2) EMITB(0x39)                 \
        MRM(0x01,    MOD(XS), REG(XS))

#define mz1ox_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVX(RXB(XD),       0,    0x00, K, 2, 2) EMITB(0x38)                 \
        MRM(REG(XD),    0x03,    0x01)

#endif /* (RT_512X4 == 2) */

#undef  K
#define K 2

#define XmmG    0x10, 0x03, EMPTY
#define XmmO    0x18, 0x03, EMPTY

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmox_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmix_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movox_ld(XD, MS, DS)                                                \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

#define movox_st(XS, MD, DD)                                                \
    ADR EVX(0,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR EVX(1,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VZL(DD)), EMPTY)                                 \
    ADR EVX(2,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VSL(DD)), EMPTY)                                 \
    ADR EVX(3,       RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VTL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvox_rr(XG, XS)                                                    \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(0,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1ox_rm(Xmm8, Mebp, inf_GPC07)                                     \
        EKX(1,             1,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1ox_rm(XmmG, Mebp, inf_GPC07)                                     \
        EKX(2,             2,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1ox_rm(XmmO, Mebp, inf_GPC07)                                     \
        EKX(3,             3,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvox_ld(XG, MS, DS)                                                \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(0,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        ck1ox_rm(Xmm8, Mebp, inf_GPC07)                                     \
    ADR EKX(1,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
        ck1ox_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKX(2,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
        ck1ox_rm(XmmO, Mebp, inf_GPC07)                                     \
    ADR EKX(3,       RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

#define mmvox_st(XS, MG, DG)                                                \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(0,       RXB(MG),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VAL(DG)), EMPTY)                                 \
        ck1ox_rm(Xmm8, Mebp, inf_GPC07)                                     \
    ADR EKX(1,       RXB(MG),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VZL(DG)), EMPTY)                                 \
        ck1ox_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKX(2,       RXB(MG),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VSL(DG)), EMPTY)                                 \
        ck1ox_rm(XmmO, Mebp, inf_GPC07)                                     \
    ADR EKX(3,       RXB(MG),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VTL(DG)), EMPTY)

#if (RT_512X4 < 2)

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        notox_rr(W(XD), W(XS))                                              \
        orrox_rr(W(XD), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        notox_rr(W(XD), W(XS))                                              \
        orrox_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorox_rr(XG, XS)                                                    \
        xorox3rr(W(XG), W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorox3ld(W(XG), W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

#else /* RT_512X4 >= 2 */

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        notox_rr(W(XD), W(XS))                                              \
        orrox_rr(W(XD), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        notox_rr(W(XD), W(XS))                                              \
        orrox_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorox_rr(XG, XS)                                                    \
        xorox3rr(W(XG), W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorox3ld(W(XG), W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

#endif /* RT_512X4 >= 2 */

/* not (G = ~G), (D = ~S) */

#define notox_rx(XG)                                                        \
        notox_rr(W(XG), W(XG))

#define notox_rr(XD, XS)                                                    \
        annox3ld(W(XD), W(XS), Mebp, inf_GPC07)

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negos_rx(XG)                                                        \
        negos_rr(W(XG), W(XG))

#define negos_rr(XD, XS)                                                    \
        xorox3ld(W(XD), W(XS), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addos_rr(XG, XS)                                                    \
        addos3rr(W(XG), W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addos3ld(W(XG), W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subos_rr(XG, XS)                                                    \
        subos3rr(W(XG), W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subos3ld(W(XG), W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulos_rr(XG, XS)                                                    \
        mulos3rr(W(XG), W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulos3ld(W(XG), W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divos_rr(XG, XS)                                                    \
        divos3rr(W(XG), W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divos3ld(W(XG), W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqros_ld(XD, MS, DS)                                                \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RCP == 2

#define rceos_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        addos_rr(W(XG), W(XG))                                              \
        subos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0

#define rseos_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RSQ == 2

#define rseos_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        subos_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulos_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
    ADR EVX(0,             0, REG(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(1,             1, REH(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(2,             2, REI(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
    ADR EVX(0,             0, REG(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(1,             1, REH(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(2,             2, REI(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minos_rr(XG, XS)                                                    \
        minos3rr(W(XG), W(XG), W(XS))

#define minos_ld(XG, MS, DS)                                                \
        minos3ld(W(XG), W(XG), W(MS), W(DS))

#define minos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxos_rr(XG, XS)                                                    \
        maxos3rr(W(XG), W(XG), W(XS))

#define maxos_ld(XG, MS, DS)                                                \
        maxos3ld(W(XG), W(XG), W(MS), W(DS))

#define maxos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqos_rr(XG, XS)                                                    \
        ceqos3rr(W(XG), W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqos3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define ceqos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x00))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x00))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x00))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x00))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneos_rr(XG, XS)                                                    \
        cneos3rr(W(XG), W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cneos3ld(W(XG), W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cneos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x04))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x04))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x04))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x04))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltos_rr(XG, XS)                                                    \
        cltos3rr(W(XG), W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltos3ld(W(XG), W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cltos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x01))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x01))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x01))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x01))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleos_rr(XG, XS)                                                    \
        cleos3rr(W(XG), W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        cleos3ld(W(XG), W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cleos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x02))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x02))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x02))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x02))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtos_rr(XG, XS)                                                    \
        cgtos3rr(W(XG), W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cgtos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x06))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x06))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x06))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x06))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeos_rr(XG, XS)                                                    \
        cgeos3rr(W(XG), W(XG), W(XS))

#define cgeos_ld(XG, MS, DS)                                                \
        cgeos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeos3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cgeos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x05))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x05))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x05))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x05))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_2K8    0x0000   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_2K8    0xFFFF   /*  all satisfy the condition */

#define mk1wx_rx(RD)         /* not portable, do not use outside */         \
        VEX(RXB(RD),       0,    0x00, 0, 0, 1) EMITB(0x93)                 \
        MRM(REG(RD),    0x03,    0x01)

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1ox_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(1,             0) EMITB(0x8B)                                   \
        MRM(0x07,       0x03, 0x00)                                         \
        ck1ox_rm(V(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(1,             0)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##32_2K8 >> 15) << 1)))  \
        MRM(0x07,       0x03, 0x00)                                         \
        ck1ox_rm(X(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(1,             0)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##32_2K8 >> 15) << 1)))  \
        MRM(0x07,       0x03, 0x00)                                         \
        ck1ox_rm(Z(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##32_2K8 >> 15) << 1)))  \
        MRM(0x00,       0x03, 0x07)                                         \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_2K8))                     \
        jeqxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(0,             0,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVX(1,             1,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVX(2,             2,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVX(3,             3,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x03))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x03))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x03))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(0,             0,    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(1,       RXB(MS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EVX(2,       RXB(MS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EVX(3,       RXB(MS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EVX(0,             0,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVX(1,             1,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVX(2,             2,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVX(3,             3,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x02))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x02))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x02))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        ERX(0,             0,    0x00, 2, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(1,             1,    0x00, 2, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(2,             2,    0x00, 2, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(3,             3,    0x00, 2, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cvpos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EVX(0,             0,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVX(1,             1,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVX(2,             2,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVX(3,             3,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x01))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x01))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x01))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        ERX(0,             0,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(1,             1,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(2,             2,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(3,             3,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cvmos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EVX(0,             0,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVX(1,             1,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVX(2,             2,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVX(3,             3,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVX(0,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x00))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x00))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x00))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        cvtos_rr(W(XD), W(XS))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        cvtos_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVX(1,             1,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVX(2,             2,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVX(3,             3,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XD, MS, DS)                                                \
    ADR EVX(0,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR EVX(1,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x04))                           \
    ADR EVX(2,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMITB(0x04))                           \
    ADR EVX(3,       RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMITB(0x04))

#define cvtos_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtos_ld(XD, MS, DS)                                                \
    ADR EVX(0,       RXB(MS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(1,       RXB(MS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EVX(2,       RXB(MS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EVX(3,       RXB(MS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvton_rr(XD, XS)                                                    \
        EVX(0,             0,    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(1,             1,    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(2,             2,    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(3,             3,    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvton_ld(XD, MS, DS)                                                \
    ADR EVX(0,       RXB(MS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(1,       RXB(MS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)                                 \
    ADR EVX(2,       RXB(MS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VSL(DS)), EMPTY)                                 \
    ADR EVX(3,       RXB(MS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VTL(DS)), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EVX(0,             0,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVX(1,             1,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVX(2,             2,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVX(3,             3,    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XD, XS, mode)                                              \
        ERX(0,             0, 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(1,             1, 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(2,             2, 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(3,             3, 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)\
        MRM(REG(XD), MOD(XS), REG(XS))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addox_rr(XG, XS)                                                    \
        addox3rr(W(XG), W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addox3ld(W(XG), W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subox_rr(XG, XS)                                                    \
        subox3rr(W(XG), W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subox3ld(W(XG), W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulox_rr(XG, XS)                                                    \
        mulox3rr(W(XG), W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulox3ld(W(XG), W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x40)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlox3ri(W(XG), W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlox3ld(W(XG), W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        EVX(0,             0, REG(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             1, REH(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             2, REI(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             3, REJ(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrox3ri(W(XG), W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrox3ld(W(XG), W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        EVX(0,             0, REG(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             1, REH(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             2, REI(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             3, REJ(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shron_ri(XG, IS)                                                    \
        shron3ri(W(XG), W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shron3ld(W(XG), W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        EVX(0,             0, REG(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             1, REH(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             2, REI(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        EVX(0,             3, REJ(XD), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shron3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlox3rr(W(XG), W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlox3ld(W(XG), W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrox3rr(W(XG), W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrox3ld(W(XG), W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svron3rr(W(XG), W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svron3ld(W(XG), W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minox_rr(XG, XS)                                                    \
        minox3rr(W(XG), W(XG), W(XS))

#define minox_ld(XG, MS, DS)                                                \
        minox3ld(W(XG), W(XG), W(MS), W(DS))

#define minox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x3B)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minon_rr(XG, XS)                                                    \
        minon3rr(W(XG), W(XG), W(XS))

#define minon_ld(XG, MS, DS)                                                \
        minon3ld(W(XG), W(XG), W(MS), W(DS))

#define minon3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minon3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x39)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxox_rr(XG, XS)                                                    \
        maxox3rr(W(XG), W(XG), W(XS))

#define maxox_ld(XG, MS, DS)                                                \
        maxox3ld(W(XG), W(XG), W(MS), W(DS))

#define maxox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x3F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxon_rr(XG, XS)                                                    \
        maxon3rr(W(XG), W(XG), W(XS))

#define maxon_ld(XG, MS, DS)                                                \
        maxon3ld(W(XG), W(XG), W(MS), W(DS))

#define maxon3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(1,             1, REH(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(2,             2, REI(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(3,             3, REJ(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxon3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(1,       RXB(MT), REH(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)                                 \
    ADR EVX(2,       RXB(MT), REI(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMPTY)                                 \
    ADR EVX(3,       RXB(MT), REJ(XS), K, 1, 2) EMITB(0x3D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqox_rr(XG, XS)                                                    \
        ceqox3rr(W(XG), W(XG), W(XS))

#define ceqox_ld(XG, MS, DS)                                                \
        ceqox3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define ceqox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x00))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x00))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x00))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x00))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneox_rr(XG, XS)                                                    \
        cneox3rr(W(XG), W(XG), W(XS))

#define cneox_ld(XG, MS, DS)                                                \
        cneox3ld(W(XG), W(XG), W(MS), W(DS))

#define cneox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cneox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x04))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x04))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x04))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x04))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltox_rr(XG, XS)                                                    \
        cltox3rr(W(XG), W(XG), W(XS))

#define cltox_ld(XG, MS, DS)                                                \
        cltox3ld(W(XG), W(XG), W(MS), W(DS))

#define cltox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cltox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x01))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x01))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x01))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x01))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define clton_rr(XG, XS)                                                    \
        clton3rr(W(XG), W(XG), W(XS))

#define clton_ld(XG, MS, DS)                                                \
        clton3ld(W(XG), W(XG), W(MS), W(DS))

#define clton3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define clton3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x01))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x01))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x01))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x01))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleox_rr(XG, XS)                                                    \
        cleox3rr(W(XG), W(XG), W(XS))

#define cleox_ld(XG, MS, DS)                                                \
        cleox3ld(W(XG), W(XG), W(MS), W(DS))

#define cleox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cleox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x02))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x02))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x02))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x02))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleon_rr(XG, XS)                                                    \
        cleon3rr(W(XG), W(XG), W(XS))

#define cleon_ld(XG, MS, DS)                                                \
        cleon3ld(W(XG), W(XG), W(MS), W(DS))

#define cleon3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cleon3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x02))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x02))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x02))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x02))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtox_rr(XG, XS)                                                    \
        cgtox3rr(W(XG), W(XG), W(XS))

#define cgtox_ld(XG, MS, DS)                                                \
        cgtox3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cgtox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x06))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x06))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x06))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x06))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgton_rr(XG, XS)                                                    \
        cgton3rr(W(XG), W(XG), W(XS))

#define cgton_ld(XG, MS, DS)                                                \
        cgton3ld(W(XG), W(XG), W(MS), W(DS))

#define cgton3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cgton3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x06))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x06))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x06))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x06))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeox_rr(XG, XS)                                                    \
        cgeox3rr(W(XG), W(XG), W(XS))

#define cgeox_ld(XG, MS, DS)                                                \
        cgeox3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeox3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cgeox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x05))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x05))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x05))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1E)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x05))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeon_rr(XG, XS)                                                    \
        cgeon3rr(W(XG), W(XG), W(XS))

#define cgeon_ld(XG, MS, DS)                                                \
        cgeon3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeon3rr(XD, XS, XT)                                                \
        EVX(0,             0, REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             1, REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             2, REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
        EVX(0,             3, REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

#define cgeon3ld(XD, XS, MT, DT)                                            \
    ADR EVX(0,       RXB(MT), REG(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x05))                           \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REH(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMITB(0x05))                           \
        mz1ox_ld(V(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REI(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VSL(DT)), EMITB(0x05))                           \
        mz1ox_ld(X(XD), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MT), REJ(XS), K, 1, 3) EMITB(0x1F)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VTL(DT)), EMITB(0x05))                           \
        mz1ox_ld(Z(XD), Mebp, inf_GPC07)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#undef  sregs_sa
#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        VEX(0,             0,    0x00, 0, 0, 1) EMITB(0x91)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_st(Redx)                                                      \
        stack_st(Recx)                                                      \
        stack_st(Rebx)                                                      \
        stack_st(Reax)                                                      \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        stack_ld(Reax)                                                      \
        andwxZri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
        EMITB(0x74) EMITB(0x05)                                             \
        VEW(0,             0,    0x00, 0, 0, 1) EMITB(0x91)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_ld(Rebx)                                                      \
        stack_ld(Recx)                                                      \
        stack_ld(Redx)

#undef  sregs_la
#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IM(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        VEX(0,             0,    0x00, 0, 0, 1) EMITB(0x90)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_st(Redx)                                                      \
        stack_st(Recx)                                                      \
        stack_st(Rebx)                                                      \
        stack_st(Reax)                                                      \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        stack_ld(Reax)                                                      \
        andwxZri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
        EMITB(0x74) EMITB(0x05)                                             \
        VEW(0,             0,    0x00, 0, 0, 1) EMITB(0x90)                 \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_ld(Rebx)                                                      \
        stack_ld(Recx)                                                      \
        stack_ld(Redx)

#endif /* RT_512X4 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_512X4V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

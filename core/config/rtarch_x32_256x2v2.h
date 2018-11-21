/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_256X2V2_H
#define RT_RTARCH_X32_256X2V2_H

#include "rtarch_x64.h"

#define RT_SIMD_REGS            8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_256x2v2.h: Implementation of x86_64 fp32 AVX1/2 instruction pairs.
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

#if (RT_256X2 >= 1 && RT_256X2 <= 2)

#ifndef RT_RTARCH_X64_256X1V2_H
#undef  RT_256X1
#define RT_256X1  RT_256X2
#include "rtarch_x64_256x1v2.h"
#endif /* RT_RTARCH_X64_256X1V2_H */

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
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movox_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

#define movox_st(XS, MD, DD)                                                \
    ADR VEX(0,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR VEX(1,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VXL(DD)), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvox_rr(XG, XS)                                                    \
    ADR VEX(0,             0, REG(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
    ADR VEX(1,             1, REH(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x80))

#define mmvox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x80))

#define mmvox_st(XS, MG, DG)                                                \
    ADR VEX(0,       RXB(MG),    0x00, 1, 1, 2) EMITB(0x2E)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VAL(DG)), EMPTY)                                 \
    ADR VEX(1,       RXB(MG),    0x08, 1, 1, 2) EMITB(0x2E)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VXL(DG)), EMPTY)

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorox_rr(XG, XS)                                                    \
        xorox3rr(W(XG), W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorox3ld(W(XG), W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

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

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addos_rr(XG, XS)                                                    \
        addos3rr(W(XG), W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addos3ld(W(XG), W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subos_rr(XG, XS)                                                    \
        subos3rr(W(XG), W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subos3ld(W(XG), W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulos_rr(XG, XS)                                                    \
        mulos3rr(W(XG), W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulos3ld(W(XG), W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divos_rr(XG, XS)                                                    \
        divos3rr(W(XG), W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divos3ld(W(XG), W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqros_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x53)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x53)                 \
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

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x52)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x52)                 \
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

#if (RT_256X2 < 2)

#define prmox_rr(XD, XS, IT) /* not portable, do not use outside */         \
        VEX(0,             0, REG(XD), 1, 1, 3) EMITB(0x06)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        VEX(1,             1, REH(XD), 1, 1, 3) EMITB(0x06)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define addzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x00,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x05,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))


#define mivox_st(XS, MD, DD) /* not portable, do not use outside */         \
    ADR VEX(0,       RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR VEX(1,       RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), EMITW(VXL(DD)), EMPTY)

#define cvyos_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvyos_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

#define cvxqs_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(0,             0,    0x00, 1, 1, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 1, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_rr(W(XS), W(XT))                                              \
        addos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_ld(W(XS), W(MT), W(DT))                                       \
        addos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvyos_rr(W(XS), W(XT))                                              \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        addqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x00))                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvyos_rr(W(XS), W(XT))                                              \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        addqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x10))                              \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvyos_ld(W(XS), W(MT), W(DT))                                       \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        addqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x00))                              \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvyos_ld(W(XS), W(MT), W(DT))                                       \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        addqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x10))                              \
        subzm_ri(W(MT), IC(0x10))                  /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_rr(W(XS), W(XT))                                              \
        subos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_ld(W(XS), W(MT), W(DT))                                       \
        subos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvyos_rr(W(XS), W(XT))                                              \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        subqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x00))                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvyos_rr(W(XS), W(XT))                                              \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        subqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x10))                              \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvyos_ld(W(XS), W(MT), W(DT))                                       \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        subqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x00))                              \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))     /* 2nd-pass -> */        \
        cvyos_ld(W(XS), W(MT), W(DT))                                       \
        mulqs_rr(W(XS), W(XG))                                              \
        cvyos_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        subqs_rr(W(XG), W(XS))                                              \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR02(0x10))                              \
        subzm_ri(W(MT), IC(0x10))                  /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_256X2 >= 2, AVX2 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
    ADR VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
    ADR VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_256X2 >= 2, AVX2 */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minos_rr(XG, XS)                                                    \
        minos3rr(W(XG), W(XG), W(XS))

#define minos_ld(XG, MS, DS)                                                \
        minos3ld(W(XG), W(XG), W(MS), W(DS))

#define minos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxos_rr(XG, XS)                                                    \
        maxos3rr(W(XG), W(XG), W(XS))

#define maxos_ld(XG, MS, DS)                                                \
        maxos3ld(W(XG), W(XG), W(MS), W(DS))

#define maxos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqos_rr(XG, XS)                                                    \
        ceqos3rr(W(XG), W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqos3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x00))                           \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMITB(0x00))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneos_rr(XG, XS)                                                    \
        cneos3rr(W(XG), W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cneos3ld(W(XG), W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x04))                           \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMITB(0x04))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltos_rr(XG, XS)                                                    \
        cltos3rr(W(XG), W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltos3ld(W(XG), W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x01))                           \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMITB(0x01))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleos_rr(XG, XS)                                                    \
        cleos3rr(W(XG), W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        cleos3ld(W(XG), W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x02))                           \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMITB(0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtos_rr(XG, XS)                                                    \
        cgtos3rr(W(XG), W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x06))                           \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMITB(0x06))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeos_rr(XG, XS)                                                    \
        cgeos3rr(W(XG), W(XG), W(XS))

#define cgeos_ld(XG, MS, DS)                                                \
        cgeos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMITB(0x05))                           \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMITB(0x05))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_512    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_512    0xFF     /*  all satisfy the condition */

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(1,             0) EMITB(0x8B)                                   \
        MRM(0x07,       0x03, 0x00)                                         \
        VEX(0,             1,    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##32_512 >> 7) << 1)))   \
        MRM(0x00,       0x03, 0x07)                                         \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_512))                     \
        jeqxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x03))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(0,             0,    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x02))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x01))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x00))

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
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x04))

#define cvtos_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtos_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvton_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvton_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)                                        \
        cvzos_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

#if (RT_256X2 < 2)

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addox_rr(XG, XS)                                                    \
        addox3rr(W(XG), W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addox3ld(W(XG), W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define addox3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        addix_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subox_rr(XG, XS)                                                    \
        subox3rr(W(XG), W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subox3ld(W(XG), W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define subox3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        subix_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlox3ri(W(XG), W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlox3ld(W(XG), W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlix3ri(W(XD), W(XS), W(IT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shlix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define shlox3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shlix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shlix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrox3ri(W(XG), W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrox3ld(W(XG), W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrix3ri(W(XD), W(XS), W(IT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrix_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define shrox3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shron_ri(XG, IS)                                                    \
        shron3ri(W(XG), W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shron3ld(W(XG), W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrin3ri(W(XD), W(XS), W(IT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrin_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrin_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrin_ri(W(XD), W(IT))                                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define shron3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrin3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        shrin_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrin_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        shrin_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlox3rr(W(XG), W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlox3ld(W(XG), W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define svlox3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrox3rr(W(XG), W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrox3ld(W(XG), W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define svrox3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svron3rr(W(XG), W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svron3ld(W(XG), W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#define svron3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_256X2 >= 2, AVX2 */

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addox_rr(XG, XS)                                                    \
        addox3rr(W(XG), W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addox3ld(W(XG), W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subox_rr(XG, XS)                                                    \
        subox3rr(W(XG), W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subox3ld(W(XG), W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlox3ri(W(XG), W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlox3ld(W(XG), W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        VEX(0,             0, REG(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))                        \
        VEX(0,             1, REH(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))

#define shlox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrox3ri(W(XG), W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrox3ld(W(XG), W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        VEX(0,             0, REG(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))                        \
        VEX(0,             1, REH(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))

#define shrox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shron_ri(XG, IS)                                                    \
        shron3ri(W(XG), W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shron3ld(W(XG), W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        VEX(0,             0, REG(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))                        \
        VEX(0,             1, REH(XD), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT) & 0x1F))

#define shron3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlox3rr(W(XG), W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlox3ld(W(XG), W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrox3rr(W(XG), W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrox3ld(W(XG), W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svron3rr(W(XG), W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svron3ld(W(XG), W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_256X2 >= 2, AVX2 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#undef  sregs_sa
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
        movox_st(Xmm7, Oeax, PLAIN)

#undef  sregs_la
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
        movox_ld(Xmm7, Oeax, PLAIN)

#endif /* RT_256X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_256X2V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

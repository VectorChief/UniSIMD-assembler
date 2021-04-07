/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_128X1V4_H
#define RT_RTARCH_X86_128X1V4_H

#include "rtarch_x86.h"

#define RT_SIMD_REGS_128        8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86_128x1v4.h: Implementation of x86 fp32 SSE1/2/4 instructions.
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

#if (RT_128X1 >= 1 && RT_128X1 <= 4)

/* mandatory escape prefix for some opcodes */
#define xF2                                                                 \
        EMITB(0xF2)

/* mandatory escape prefix for some opcodes */
#define xF3                                                                 \
        EMITB(0xF3)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x03, EMPTY
#define Xmm1    0x01, 0x03, EMPTY
#define Xmm2    0x02, 0x03, EMPTY
#define Xmm3    0x03, 0x03, EMPTY
#define Xmm4    0x04, 0x03, EMPTY
#define Xmm5    0x05, 0x03, EMPTY
#define Xmm6    0x06, 0x03, EMPTY
#define Xmm7    0x07, 0x03, EMPTY

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
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movix_ld(XD, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movix_st(XS, MD, DD)                                                \
        EMITB(0x0F) EMITB(0x29)                                             \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#if (RT_128X1 < 4)

#define mmvix_rr(XG, XS)                                                    \
        andix_rr(W(XS), Xmm0)                                               \
        annix_rr(Xmm0, W(XG))                                               \
        orrix_rr(Xmm0, W(XS))                                               \
        movix_rr(W(XG), Xmm0)

#define mmvix_ld(XG, MS, DS)                                                \
        notix_rx(Xmm0)                                                      \
        andix_rr(W(XG), Xmm0)                                               \
        annix_ld(Xmm0, W(MS), W(DS))                                        \
        orrix_rr(W(XG), Xmm0)

#else /* RT_128X1 >= 4 */

#define mmvix_rr(XG, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0x38) EMITB(0x14)                                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvix_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0x38) EMITB(0x14)                                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_128X1 >= 4 */

#define mmvix_st(XS, MG, DG)                                                \
        andix_rr(W(XS), Xmm0)                                               \
        annix_ld(Xmm0, W(MG), W(DG))                                        \
        orrix_rr(Xmm0, W(XS))                                               \
        movix_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andix_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andix_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        andix_rr(W(XD), W(XT))

#define andix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        andix_ld(W(XD), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annix_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annix_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define annix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        annix_rr(W(XD), W(XT))

#define annix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        annix_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrix_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrix_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        orrix_rr(W(XD), W(XT))

#define orrix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        orrix_ld(W(XD), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornix_rr(XG, XS)                                                    \
        notix_rx(W(XG))                                                     \
        orrix_rr(W(XG), W(XS))

#define ornix_ld(XG, MS, DS)                                                \
        notix_rx(W(XG))                                                     \
        orrix_ld(W(XG), W(MS), W(DS))

#define ornix3rr(XD, XS, XT)                                                \
        notix_rr(W(XD), W(XS))                                              \
        orrix_rr(W(XD), W(XT))

#define ornix3ld(XD, XS, MT, DT)                                            \
        notix_rr(W(XD), W(XS))                                              \
        orrix_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorix_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorix_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        xorix_rr(W(XD), W(XT))

#define xorix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        xorix_ld(W(XD), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notix_rx(XG)                                                        \
        annix_ld(W(XG), Mebp, inf_GPC07)

#define notix_rr(XD, XS)                                                    \
        movix_rr(W(XD), W(XS))                                              \
        notix_rx(W(XD))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negis_rx(XG)                                                        \
        xorix_ld(W(XG), Mebp, inf_GPC06_32)

#define negis_rr(XD, XS)                                                    \
        movix_rr(W(XD), W(XS))                                              \
        negis_rx(W(XD))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        addis_rr(W(XD), W(XT))

#define addis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        addis_ld(W(XD), W(MT), W(DT))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_128X1 >= 4)

#undef  adpis_rr
#define adpis_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
    xF2 EMITB(0x0F) EMITB(0x7C)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#undef  adpis_ld
#define adpis_ld(XG, MS, DS)                                                \
    xF2 EMITB(0x0F) EMITB(0x7C)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#undef  adpis3rr
#define adpis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        adpis_rr(W(XD), W(XT))

#undef  adpis3ld
#define adpis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        adpis_ld(W(XD), W(MT), W(DT))

#endif /* RT_128X1 >= 4 */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        subis_rr(W(XD), W(XT))

#define subis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        subis_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        mulis_rr(W(XD), W(XT))

#define mulis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        mulis_ld(W(XD), W(MT), W(DT))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        divis_rr(W(XD), W(XT))

#define divis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        divis_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqris_rr(XD, XS)                                                    \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqris_ld(XD, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITB(0x0F) EMITB(0x53)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsis_rr(XG, XS) /* destroys XS */                                  \
        mulis_rr(W(XS), W(XG))                                              \
        mulis_rr(W(XS), W(XG))                                              \
        addis_rr(W(XG), W(XG))                                              \
        subis_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITB(0x0F) EMITB(0x52)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssis_rr(XG, XS) /* destroys XS */                                  \
        mulis_rr(W(XS), W(XG))                                              \
        mulis_rr(W(XS), W(XG))                                              \
        subis_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulis_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulis_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmais_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_rr(W(XS), W(XT))                                              \
        addis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmais_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_ld(W(XS), W(MT), W(DT))                                       \
        addis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmais_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmais_rx(W(XG))

#define fmais_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XS), W(MT), W(DT))                                       \
        movix_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmais_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmais_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmais_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmais_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XS), W(MT), W(DT))                                       \
        movix_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmais_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmais_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        movix_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x0C))                                    \
        addws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x08))                                    \
        addws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x04))                                    \
        addws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movix_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsis_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_rr(W(XS), W(XT))                                              \
        subis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_ld(W(XS), W(MT), W(DT))                                       \
        subis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsis_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsis_rx(W(XG))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XS), W(MT), W(DT))                                       \
        movix_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsis_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsis_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsis_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XS), W(MT), W(DT))                                       \
        movix_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsis_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsis_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        movix_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x0C))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x08))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x04))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movix_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define minis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        minis_rr(W(XD), W(XT))

#define minis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        minis_ld(W(XD), W(MT), W(DT))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        maxis_rr(W(XD), W(XT))

#define maxis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        maxis_ld(W(XD), W(MT), W(DT))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define ceqis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        ceqis_rr(W(XD), W(XT))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        ceqis_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cneis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        cneis_rr(W(XD), W(XT))

#define cneis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        cneis_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cltis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        cltis_rr(W(XD), W(XT))

#define cltis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        cltis_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cleis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        cleis_rr(W(XD), W(XT))

#define cleis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        cleis_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgtis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        cgtis_rr(W(XD), W(XT))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        cgtis_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeis_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeis_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

#define cgeis3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        cgeis_rr(W(XD), W(XT))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        cgeis_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128    0x0F     /*  all satisfy the condition */

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITB(0x0F) EMITB(0x50)                                             \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##32_128))                     \
        jeqxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

#if (RT_128X1 < 2)

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        cvzis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        cvzis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x0C))                                    \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        cvpis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnis_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnis_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        cvmis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnis_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnis_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        cvnis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        cvnis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x0C))                                    \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        cvnis_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuwn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x0C))                                    \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#else /* RT_128X1 >= 2 */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128X1 < 4)

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        cvzis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        cvzis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#else /* RT_128X1 >= 4 */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#endif /* RT_128X1 >= 4 */

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
    xF3 EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
    xF3 EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128X1 < 4)

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        cvpis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#else /* RT_128X1 >= 4 */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        rnpis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

#endif /* RT_128X1 >= 4 */

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128X1 < 4)

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        cvmis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#else /* RT_128X1 >= 4 */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        rnmis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

#endif /* RT_128X1 >= 4 */

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128X1 < 4)

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        cvnis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        cvnis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#else /* RT_128X1 >= 4 */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#endif /* RT_128X1 >= 4 */

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        cvtis_rr(W(XD), W(XS))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        cvtis_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        cvtin_rr(W(XD), W(XS))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        cvtin_ld(W(XD), W(MS), W(DS))

#endif /* RT_128X1 >= 2 */

#if (RT_128X1 < 2)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        cvtis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rndis_ld(XD, MS, DS)                                                \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvtis_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnis_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtis_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        cvtis_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtin_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnin_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtin_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        cvtin_rr(W(XD), W(XD))

#else /* RT_128X1 >= 2 */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128X1 < 4)

#define rndis_rr(XD, XS)                                                    \
        cvtis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rndis_ld(XD, MS, DS)                                                \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#else /* RT_128X1 >= 4 */

#define rndis_rr(XD, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndis_ld(XD, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#endif /* RT_128X1 >= 4 */

#define cvtis_rr(XD, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtis_ld(XD, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtin_rr(XD, XS)                                                    \
        EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtin_ld(XD, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_128X1 >= 2 */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128X1 < 4)

#define rnris_rr(XD, XS, mode)                                              \
        cvris_rr(W(XD), W(XS), mode)                                        \
        cvnin_rr(W(XD), W(XD))

#define cvris_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_128X1 >= 4 */

#define rnris_rr(XD, XS, mode)                                              \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvris_rr(XD, XS, mode)                                              \
        rnris_rr(W(XD), W(XS), mode)                                        \
        cvzis_rr(W(XD), W(XD))

#endif /* RT_128X1 >= 4 */

/************   packed single-precision integer arithmetic/shifts   ***********/

#if (RT_128X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addix_rr(XG, XS)                                                    \
        addix3rr(W(XG), W(XG), W(XS))

#define addix_ld(XG, MS, DS)                                                \
        addix3ld(W(XG), W(XG), W(MS), W(DS))

#define addix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define addix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subix_rr(XG, XS)                                                    \
        subix3rr(W(XG), W(XG), W(XS))

#define subix_ld(XG, MS, DS)                                                \
        subix3ld(W(XG), W(XG), W(MS), W(DS))

#define subix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define subix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulix_rr(XG, XS)                                                    \
        mulix3rr(W(XG), W(XG), W(XS))

#define mulix_ld(XG, MS, DS)                                                \
        mulix3ld(W(XG), W(XG), W(MS), W(DS))

#define mulix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mulix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlix_ri(XG, IS)                                                    \
        shlix3ri(W(XG), W(XG), W(IS))

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix3ld(W(XG), W(XG), W(MS), W(DS))

#define shlix3ri(XD, XS, IT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define shlix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrix_ri(XG, IS)                                                    \
        shrix3ri(W(XG), W(XG), W(IS))

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix3ld(W(XG), W(XG), W(MS), W(DS))

#define shrix3ri(XD, XS, IT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define shrix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrin_ri(XG, IS)                                                    \
        shrin3ri(W(XG), W(XG), W(IS))

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin3ld(W(XG), W(XG), W(MS), W(DS))

#define shrin3ri(XD, XS, IT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrwn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define shrin3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(MT), W(DT))                                       \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_128X1 >= 2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addix_rr(XG, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0xFE)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addix_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xFE)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        addix_rr(W(XD), W(XT))

#define addix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        addix_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subix_rr(XG, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0xFA)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subix_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xFA)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        subix_rr(W(XD), W(XT))

#define subix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        subix_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#if (RT_128X1 < 4)

#define mulix_rr(XG, XS)                                                    \
        mulix3rr(W(XG), W(XG), W(XS))

#define mulix_ld(XG, MS, DS)                                                \
        mulix3ld(W(XG), W(XG), W(MS), W(DS))

#define mulix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define mulix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_128X1 >= 4 */

#define mulix_rr(XG, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0x38) EMITB(0x40)                                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulix_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0x38) EMITB(0x40)                                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        mulix_rr(W(XD), W(XT))

#define mulix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        mulix_ld(W(XD), W(MT), W(DT))

#endif /* RT_128X1 >= 4 */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlix_ri(XG, IS)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
    ESC EMITB(0x0F) EMITB(0xF2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shlix3ri(XD, XS, IT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        shlix_ri(W(XD), W(IT))

#define shlix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        shlix_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrix_ri(XG, IS)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
    ESC EMITB(0x0F) EMITB(0xD2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrix3ri(XD, XS, IT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        shrix_ri(W(XD), W(IT))

#define shrix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        shrix_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrin_ri(XG, IS)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
    ESC EMITB(0x0F) EMITB(0xE2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrin3ri(XD, XS, IT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        shrin_ri(W(XD), W(IT))

#define shrin3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        shrin_ld(W(XD), W(MT), W(DT))

#endif /* RT_128X1 >= 2 */

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix3rr(W(XG), W(XG), W(XS))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix3ld(W(XG), W(XG), W(MS), W(DS))

#define svlix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define svlix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix3rr(W(XG), W(XG), W(XS))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix3ld(W(XG), W(XG), W(MS), W(DS))

#define svrix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define svrix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin3rr(W(XG), W(XG), W(XS))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin3ld(W(XG), W(XG), W(MS), W(DS))

#define svrin3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define svrin3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgx_ld(XD, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgx_st(XS, MD, DD)                                                \
        EMITB(0x0F) EMITB(0x29)                                             \
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

#if (RT_128X1 < 2)

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andgx_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x54)                                             \
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
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define anngx_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x55)                                             \
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
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrgx_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x56)                                             \
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
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorgx_ld(XG, MS, DS)                                                \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_rr(W(XD), W(XT))

#define xorgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_ld(W(XD), W(MT), W(DT))

#else /* RT_128X1 >= 2 */

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0xDB)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xDB)                                             \
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
    ESC EMITB(0x0F) EMITB(0xDF)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define anngx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xDF)                                             \
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
    ESC EMITB(0x0F) EMITB(0xEB)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xEB)                                             \
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
    ESC EMITB(0x0F) EMITB(0xEF)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xEF)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorgx3rr(XD, XS, XT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_rr(W(XD), W(XT))

#define xorgx3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        xorgx_ld(W(XD), W(MT), W(DT))

#endif /* RT_128X1 >= 2 */

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        anngx_ld(W(XG), Mebp, inf_GPC07)

#define notgx_rr(XD, XS)                                                    \
        movgx_rr(W(XD), W(XS))                                              \
        notgx_rx(W(XD))

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_128X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
        addgx3rr(W(XG), W(XG), W(XS))

#define addgx_ld(XG, MS, DS)                                                \
        addgx3ld(W(XG), W(XG), W(MS), W(DS))

#define addgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define addgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define addgx_xx()                                                          \
        stack_st(Reax)                                                      \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x02))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x02))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x06))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x06))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0A))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x0A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0E))                              \
        addhx_st(Reax,  Mebp, inf_SCR01(0x0E))                              \
        stack_ld(Reax)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
        subgx3rr(W(XG), W(XG), W(XS))

#define subgx_ld(XG, MS, DS)                                                \
        subgx3ld(W(XG), W(XG), W(MS), W(DS))

#define subgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        subgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define subgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        subgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define subgx_xx()                                                          \
        stack_st(Reax)                                                      \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x02))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x02))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x06))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x06))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0A))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x0A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0E))                              \
        subhx_st(Reax,  Mebp, inf_SCR01(0x0E))                              \
        stack_ld(Reax)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
        mulgx3rr(W(XG), W(XG), W(XS))

#define mulgx_ld(XG, MS, DS)                                                \
        mulgx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define mulgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define mulgx_xx()                                                          \
        stack_st(Reax)                                                      \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x02))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x02))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x02))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x04))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x06))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x06))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x06))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0A))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x0A))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x0A))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        movhx_ld(Reax,  Mebp, inf_SCR02(0x0E))                              \
        mulhx_ld(Reax,  Mebp, inf_SCR01(0x0E))                              \
        movhx_st(Reax,  Mebp, inf_SCR01(0x0E))                              \
        stack_ld(Reax)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)                                                    \
        shlgx3ri(W(XG), W(XG), W(IS))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgx3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlhx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shlhx_mi(Mebp,  inf_SCR01(0x02), W(IT))                             \
        shlhx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shlhx_mi(Mebp,  inf_SCR01(0x06), W(IT))                             \
        shlhx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shlhx_mi(Mebp,  inf_SCR01(0x0A), W(IT))                             \
        shlhx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shlhx_mi(Mebp,  inf_SCR01(0x0E), W(IT))                             \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  W(MT), W(DT))                                       \
        shlhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shlhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shlhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shlhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shlhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)                                                    \
        shrgx3ri(W(XG), W(XG), W(IS))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgx3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrhx_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrhx_mi(Mebp,  inf_SCR01(0x02), W(IT))                             \
        shrhx_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrhx_mi(Mebp,  inf_SCR01(0x06), W(IT))                             \
        shrhx_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrhx_mi(Mebp,  inf_SCR01(0x0A), W(IT))                             \
        shrhx_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shrhx_mi(Mebp,  inf_SCR01(0x0E), W(IT))                             \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  W(MT), W(DT))                                       \
        shrhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)                                                    \
        shrgn3ri(W(XG), W(XG), W(IS))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgn3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrhn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrhn_mi(Mebp,  inf_SCR01(0x02), W(IT))                             \
        shrhn_mi(Mebp,  inf_SCR01(0x04), W(IT))                             \
        shrhn_mi(Mebp,  inf_SCR01(0x06), W(IT))                             \
        shrhn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        shrhn_mi(Mebp,  inf_SCR01(0x0A), W(IT))                             \
        shrhn_mi(Mebp,  inf_SCR01(0x0C), W(IT))                             \
        shrhn_mi(Mebp,  inf_SCR01(0x0E), W(IT))                             \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgn3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  W(MT), W(DT))                                       \
        shrhn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrhn_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrhn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrhn_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrhn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrhn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrhn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrhn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_128X1 >= 2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
    ESC EMITB(0x0F) EMITB(0xFD)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xFD)                                             \
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
    ESC EMITB(0x0F) EMITB(0xDD)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xDD)                                             \
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
    ESC EMITB(0x0F) EMITB(0xED)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define adsgn_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xED)                                             \
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
    ESC EMITB(0x0F) EMITB(0xF9)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xF9)                                             \
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
    ESC EMITB(0x0F) EMITB(0xD9)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xD9)                                             \
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
    ESC EMITB(0x0F) EMITB(0xE9)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xE9)                                             \
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
    ESC EMITB(0x0F) EMITB(0xD5)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulgx_ld(XG, MS, DS)                                                \
    ESC EMITB(0x0F) EMITB(0xD5)                                             \
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
    ESC EMITB(0x0F) EMITB(0x71)                                             \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
    ESC EMITB(0x0F) EMITB(0xF1)                                             \
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
    ESC EMITB(0x0F) EMITB(0x71)                                             \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
    ESC EMITB(0x0F) EMITB(0xD1)                                             \
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
    ESC EMITB(0x0F) EMITB(0x71)                                             \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
    ESC EMITB(0x0F) EMITB(0xE1)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrgn3ri(XD, XS, IT)                                                \
        movgx_rr(W(XD), W(XS))                                              \
        shrgn_ri(W(XD), W(IT))

#define shrgn3ld(XD, XS, MT, DT)                                            \
        movgx_rr(W(XD), W(XS))                                              \
        shrgn_ld(W(XD), W(MT), W(DT))

#endif /* RT_128X1 >= 2 */

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx3rr(W(XG), W(XG), W(XS))

#define svlgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define svlgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define svlgx_xx()                                                          \
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
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx3rr(W(XG), W(XG), W(XS))

#define svrgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgx_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrgx_xx()                                                          \
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
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn3rr(W(XG), W(XG), W(XS))

#define svrgn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgn3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgn_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrgn3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgn_xx()                                                          \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrgn_xx()                                                          \
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
        stack_ld(Recx)

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*********   scalar single-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movrs_rr(XD, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x10)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movrs_ld(XD, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x10)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movrs_st(XS, MD, DD)                                                \
    xF3 EMITB(0x0F) EMITB(0x11)                                             \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        addrs_rr(W(XD), W(XT))

#define addrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        addrs_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        subrs_rr(W(XD), W(XT))

#define subrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        subrs_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        mulrs_rr(W(XD), W(XT))

#define mulrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        mulrs_ld(W(XD), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        divrs_rr(W(XD), W(XT))

#define divrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        divrs_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrrs_ld(XD, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x53)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsrs_rr(XG, XS) /* destroys XS */                                  \
        mulrs_rr(W(XS), W(XG))                                              \
        mulrs_rr(W(XS), W(XG))                                              \
        addrs_rr(W(XG), W(XG))                                              \
        subrs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x52)                                             \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssrs_rr(XG, XS) /* destroys XS */                                  \
        mulrs_rr(W(XS), W(XG))                                              \
        mulrs_rr(W(XS), W(XG))                                              \
        subrs_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulrs_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulrs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmars_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_rr(W(XS), W(XT))                                              \
        addrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#define fmars_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_ld(W(XS), W(MT), W(DT))                                       \
        addrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmars_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmars_rx(W(XG))

#define fmars_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmars_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmars_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmars_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmars_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmars_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmars_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        movrs_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movrs_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsrs_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_rr(W(XS), W(XT))                                              \
        subrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_ld(W(XS), W(MT), W(DT))                                       \
        subrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsrs_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsrs_rx(W(XG))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsrs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsrs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsrs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsrs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsrs_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        movrs_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movrs_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   scalar single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define minrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        minrs_rr(W(XD), W(XT))

#define minrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        minrs_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        maxrs_rr(W(XD), W(XT))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        maxrs_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define ceqrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        ceqrs_rr(W(XD), W(XT))

#define ceqrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        ceqrs_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cners_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cners_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cners3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        cners_rr(W(XD), W(XT))

#define cners3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        cners_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cltrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        cltrs_rr(W(XD), W(XT))

#define cltrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        cltrs_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clers_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clers_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define clers3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        clers_rr(W(XD), W(XT))

#define clers3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        clers_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtrs_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtrs_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgtrs3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        cgtrs_rr(W(XD), W(XT))

#define cgtrs3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        cgtrs_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgers_rr(XG, XS)                                                    \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgers_ld(XG, MS, DS)                                                \
    xF3 EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

#define cgers3rr(XD, XS, XT)                                                \
        movrs_rr(W(XD), W(XS))                                              \
        cgers_rr(W(XD), W(XT))

#define cgers3ld(XD, XS, MT, DT)                                            \
        movrs_rr(W(XD), W(XS))                                              \
        cgers_ld(W(XD), W(MT), W(DT))

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
#define RT_SIMD_MODE_ROUNDM     0x01    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x07    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x07    /* round towards zero */

#define mxcsr_ld(MS, DS) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x02,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mxcsr_st(MD, DD) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        movwx_mi(Mebp, inf_SCR02(4), IH(RT_SIMD_MODE_##mode << 13 | 0x1F80))\
        mxcsr_ld(Mebp, inf_SCR02(4))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_ROUNDN&3)*4))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_##mode&3)*4))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_ROUNDN&3)*4))

#endif /* RT_SIMD_FAST_FCTRL */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#undef  sregs_sa
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
        movix_st(Xmm7, Oeax, PLAIN)

#undef  sregs_la
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
        movix_ld(Xmm7, Oeax, PLAIN)

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_128X1V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

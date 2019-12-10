/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_128X1V4_H
#define RT_RTARCH_X64_128X1V4_H

#include "rtarch_x32_128x1v4.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_128x1v4.h: Implementation of x86_64 fp64 SSE2/4 instructions.
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

#if (RT_128X1 == 4)

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

#define elmjx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movts_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movjx_rr(XD, XS)                                                    \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movjx_ld(XD, MS, DS)                                                \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movjx_st(XS, MD, DD)                                                \
ADR ESC REX(RXB(XS), RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#if (RT_SIMD_COMPAT_SSE < 4)

#define mmvjx_rr(XG, XS)                                                    \
        andjx_rr(W(XS), Xmm0)                                               \
        annjx_rr(Xmm0, W(XG))                                               \
        orrjx_rr(Xmm0, W(XS))                                               \
        movjx_rr(W(XG), Xmm0)

#define mmvjx_ld(XG, MS, DS)                                                \
        notjx_rx(Xmm0)                                                      \
        andjx_rr(W(XG), Xmm0)                                               \
        annjx_ld(Xmm0, W(MS), W(DS))                                        \
        orrjx_rr(W(XG), Xmm0)

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define mmvjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x15)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x15)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

#define mmvjx_st(XS, MG, DG)                                                \
        andjx_rr(W(XS), Xmm0)                                               \
        annjx_ld(Xmm0, W(MG), W(DG))                                        \
        orrjx_rr(Xmm0, W(XS))                                               \
        movjx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andjx3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        andjx_rr(W(XD), W(XT))

#define andjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        andjx_ld(W(XD), W(MT), W(DT))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define annjx3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        annjx_rr(W(XD), W(XT))

#define annjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        annjx_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrjx3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        orrjx_rr(W(XD), W(XT))

#define orrjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        orrjx_ld(W(XD), W(MT), W(DT))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornjx_rr(XG, XS)                                                    \
        notjx_rx(W(XG))                                                     \
        orrjx_rr(W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        notjx_rx(W(XG))                                                     \
        orrjx_ld(W(XG), W(MS), W(DS))

#define ornjx3rr(XD, XS, XT)                                                \
        notjx_rr(W(XD), W(XS))                                              \
        orrjx_rr(W(XD), W(XT))

#define ornjx3ld(XD, XS, MT, DT)                                            \
        notjx_rr(W(XD), W(XS))                                              \
        orrjx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorjx3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        xorjx_rr(W(XD), W(XT))

#define xorjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        xorjx_ld(W(XD), W(MT), W(DT))

/* not (G = ~G), (D = ~S) */

#define notjx_rx(XG)                                                        \
        annjx_ld(W(XG), Mebp, inf_GPC07)

#define notjx_rr(XD, XS)                                                    \
        movjx_rr(W(XD), W(XS))                                              \
        notjx_rx(W(XD))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negjs_rx(XG)                                                        \
        xorjx_ld(W(XG), Mebp, inf_GPC06_64)

#define negjs_rr(XD, XS)                                                    \
        movjx_rr(W(XD), W(XS))                                              \
        negjs_rx(W(XD))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        addjs_rr(W(XD), W(XT))

#define addjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        addjs_ld(W(XD), W(MT), W(DT))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_SIMD_COMPAT_SSE >= 4)

#undef  adpjs_rr
#define adpjs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x7C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#undef  adpjs_ld
#define adpjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x7C)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#undef  adpjs3rr
#define adpjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        adpjs_rr(W(XD), W(XT))

#undef  adpjs3ld
#define adpjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        adpjs_ld(W(XD), W(MT), W(DT))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        subjs_rr(W(XD), W(XT))

#define subjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        subjs_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define muljs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define muljs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define muljs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        muljs_rr(W(XD), W(XT))

#define muljs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        muljs_ld(W(XD), W(MT), W(DT))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        divjs_rr(W(XD), W(XT))

#define divjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        divjs_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrjs_rr(XD, XS)                                                    \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrjs_ld(XD, MS, DS)                                                \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcejs_rr(XD, XS)                                                    \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_ld(W(XD), Mebp, inf_SCR01(0))

#define rcsjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsejs_rr(XD, XS)                                                    \
        sqrjs_rr(W(XD), W(XS))                                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_ld(W(XD), Mebp, inf_SCR01(0))

#define rssjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmajs_rr(XG, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_rr(W(XS), W(XT))                                              \
        addjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_ld(W(XS), W(MT), W(DT))                                       \
        addjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmajs_rr(XG, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmajs_rx(W(XG))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XS), W(MT), W(DT))                                       \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmajs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmajs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmajs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XS), W(MT), W(DT))                                       \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmajs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmajs_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        addzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movjx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsjs_rr(XG, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_rr(W(XS), W(XT))                                              \
        subjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        muljs_ld(W(XS), W(MT), W(DT))                                       \
        subjs_rr(W(XG), W(XS))                                              \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsjs_rr(XG, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsjs_rx(W(XG))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XS), W(MT), W(DT))                                       \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsjs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsjs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsjs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XS), W(MT), W(DT))                                       \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsjs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsjs_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrzs_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x08))                                    \
        sbrzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movjx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define minjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        minjs_rr(W(XD), W(XT))

#define minjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        minjs_ld(W(XD), W(MT), W(DT))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        maxjs_rr(W(XD), W(XT))

#define maxjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        maxjs_ld(W(XD), W(MT), W(DT))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define ceqjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        ceqjs_rr(W(XD), W(XT))

#define ceqjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        ceqjs_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnejs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cnejs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cnejs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        cnejs_rr(W(XD), W(XT))

#define cnejs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        cnejs_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cltjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        cltjs_rr(W(XD), W(XT))

#define cltjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        cltjs_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clejs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clejs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define clejs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        clejs_rr(W(XD), W(XT))

#define clejs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        clejs_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgtjs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        cgtjs_rr(W(XD), W(XT))

#define cgtjs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        cgtjs_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgejs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgejs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

#define cgejs3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        cgejs_rr(W(XD), W(XT))

#define cgejs3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        cgejs_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_128    0x0F     /*  all satisfy the condition */

#define mkjjx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        REX(0,       RXB(XS)) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##64_128))                     \
        jeqxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
        cvzjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

#define cvzjs_rr(XD, XS)     /* round towards zero */                       \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x08))                                    \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvzjs_ld(XD, MS, DS) /* round towards zero */                       \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnpjs_rr(XD, XS)     /* round towards +inf */                       \
        cvpjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#define cvpjs_rr(XD, XS)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnjs_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnjs_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnpjs_rr(XD, XS)     /* round towards +inf */                       \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpjs_ld(XD, MS, DS) /* round towards +inf */                       \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpjs_rr(XD, XS)     /* round towards +inf */                       \
        rnpjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnmjs_rr(XD, XS)     /* round towards -inf */                       \
        cvmjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#define cvmjs_rr(XD, XS)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnjs_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnjs_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnmjs_rr(XD, XS)     /* round towards -inf */                       \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmjs_ld(XD, MS, DS) /* round towards -inf */                       \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmjs_rr(XD, XS)     /* round towards -inf */                       \
        rnmjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
        cvnjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
        cvnjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

#define cvnjs_rr(XD, XS)     /* round towards near */                       \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x08))                                    \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnjs_ld(XD, MS, DS) /* round towards near */                       \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        cvnjs_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnjn_rr(XD, XS)     /* round towards near */                       \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x08))                                    \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnjn_ld(XD, MS, DS) /* round towards near */                       \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#if (RT_SIMD_COMPAT_SSE < 4)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndjs_rr(XD, XS)                                                    \
        cvtjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rndjs_ld(XD, MS, DS)                                                \
        cvtjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#define cvtjs_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnjs_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtjs_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        cvtjs_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndjs_rr(XD, XS)                                                    \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndjs_ld(XD, MS, DS)                                                \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtjs_rr(XD, XS)                                                    \
        rndjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvtjs_ld(XD, MS, DS)                                                \
        rndjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtjn_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnjn_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtjn_ld(XD, MS, DS)                                                \
        movjx_ld(W(XD), W(MS), W(DS))                                       \
        cvtjn_rr(W(XD), W(XD))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_SIMD_COMPAT_SSE < 4)

#define rnrjs_rr(XD, XS, mode)                                              \
        cvrjs_rr(W(XD), W(XS), mode)                                        \
        cvnjn_rr(W(XD), W(XD))

#define cvrjs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_SIMD_COMPAT_SSE >= 4 */

#define rnrjs_rr(XD, XS, mode)                                              \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrjs_rr(XD, XS, mode)                                              \
        rnrjs_rr(W(XD), W(XS), mode)                                        \
        cvzjs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_SSE >= 4 */

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addjx3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        addjx_rr(W(XD), W(XT))

#define addjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        addjx_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subjx3rr(XD, XS, XT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        subjx_rr(W(XD), W(XT))

#define subjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        subjx_ld(W(XD), W(MT), W(DT))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shljx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xF3)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shljx3ri(XD, XS, IT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        shljx_ri(W(XD), W(IT))

#define shljx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        shljx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrjx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD3)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrjx3ri(XD, XS, IT)                                                \
        movjx_rr(W(XD), W(XS))                                              \
        shrjx_ri(W(XD), W(IT))

#define shrjx3ld(XD, XS, MT, DT)                                            \
        movjx_rr(W(XD), W(XS))                                              \
        shrjx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrjn_ri(XG, IS)                                                    \
        shrjn3ri(W(XG), W(XG), W(IS))

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrjn3ri(XD, XS, IT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IT))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IT))                             \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrjn3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MT), W(DT))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svljx3rr(W(XG), W(XG), W(XS))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svljx3ld(W(XG), W(XG), W(MS), W(DS))

#define svljx3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define svljx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjx3rr(W(XG), W(XG), W(XS))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjx3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrjx3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrjn3rr(W(XG), W(XG), W(XS))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrjn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrjn3rr(XD, XS, XT)                                                \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrjn3ld(XD, XS, MT, DT)                                            \
        movjx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XD), W(MT), W(DT))                                       \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XD), Mebp, inf_SCR01(0))

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*********   scalar double-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movts_rr(XD, XS)                                                    \
    xF2 REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x10)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movts_ld(XD, MS, DS)                                                \
ADR xF2 REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x10)                       \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movts_st(XS, MD, DD)                                                \
ADR xF2 REX(RXB(XS), RXB(MD)) EMITB(0x0F) EMITB(0x11)                       \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addts3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        addts_rr(W(XD), W(XT))

#define addts3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        addts_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subts3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        subts_rr(W(XD), W(XT))

#define subts3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        subts_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mults_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mults_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mults3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        mults_rr(W(XD), W(XT))

#define mults3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        mults_ld(W(XD), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divts3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        divts_rr(W(XD), W(XT))

#define divts3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        divts_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrts_rr(XD, XS)                                                    \
    xF2 REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrts_ld(XD, MS, DS)                                                \
ADR xF2 REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcets_rr(XD, XS)                                                    \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR01(0))

#define rcsts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsets_rr(XD, XS)                                                    \
        sqrts_rr(W(XD), W(XS))                                              \
        movts_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movts_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR01(0))

#define rssts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmats_rr(XG, XS, XT)                                                \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_rr(W(XS), W(XT))                                              \
        addts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#define fmats_ld(XG, XS, MT, DT)                                            \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_ld(W(XS), W(MT), W(DT))                                       \
        addts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmats_rr(XG, XS, XT)                                                \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmats_rx(W(XG))

#define fmats_ld(XG, XS, MT, DT)                                            \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_ld(W(XS), W(MT), W(DT))                                       \
        movts_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movts_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmats_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmats_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmats_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmats_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_ld(W(XS), W(MT), W(DT))                                       \
        movts_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movts_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmats_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmats_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        movts_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movts_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsts_rr(XG, XS, XT)                                                \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_rr(W(XS), W(XT))                                              \
        subts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_ld(W(XS), W(MT), W(DT))                                       \
        subts_rr(W(XG), W(XS))                                              \
        movts_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsts_rr(XG, XS, XT)                                                \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsts_rx(W(XG))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_ld(W(XS), W(MT), W(DT))                                       \
        movts_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movts_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsts_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsts_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsts_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movts_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movts_ld(W(XS), W(MT), W(DT))                                       \
        movts_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movts_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsts_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsts_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        movts_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movts_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   scalar double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define mints_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mints_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mints3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        mints_rr(W(XD), W(XT))

#define mints3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        mints_ld(W(XD), W(MT), W(DT))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define maxts3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        maxts_rr(W(XD), W(XT))

#define maxts3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        maxts_ld(W(XD), W(MT), W(DT))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define ceqts3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        ceqts_rr(W(XD), W(XT))

#define ceqts3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        ceqts_ld(W(XD), W(MT), W(DT))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cnets_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cnets_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cnets3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        cnets_rr(W(XD), W(XT))

#define cnets3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        cnets_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cltts3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        cltts_rr(W(XD), W(XT))

#define cltts3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        cltts_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clets_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clets_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define clets3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        clets_rr(W(XD), W(XT))

#define clets3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        clets_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgtts3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        cgtts_rr(W(XD), W(XT))

#define cgtts3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        cgtts_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgets_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgets_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

#define cgets3rr(XD, XS, XT)                                                \
        movts_rr(W(XD), W(XS))                                              \
        cgets_rr(W(XD), W(XT))

#define cgets3ld(XD, XS, MT, DT)                                            \
        movts_rr(W(XD), W(XS))                                              \
        cgets_ld(W(XD), W(MT), W(DT))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_128X1V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_128V4_H
#define RT_RTARCH_X64_128V4_H

#include "rtarch_x32_128v4.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_128v4.h: Implementation of x86_64 fp64 SSE1/2/4 instructions.
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
 * The cmdp*_** (rtbase.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via mkj**_** pseudo-ops.
 * Matching element-sized BASE subset cmdy*_** is defined in rtbase.h as well.
 *
 * Note, when using fixed-data-size 128/256-bit SIMD subsets simultaneously
 * upper 128-bit halves of full 256-bit SIMD registers may end up undefined.
 * On RISC targets they remain unchanged, while on x86-AVX they are zeroed.
 * This happens when registers written in 128-bit subset are then used/read
 * from within 256-bit subset. The same rule applies to mixing of 256/512-bit.
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

#if defined (RT_SIMD_CODE)

#if defined (RT_128) && (RT_128 >= 2 && RT_128 < 8)

/* mandatory escape prefix for some opcodes (must preceed rex) */
#define xF2                                                                 \
        EMITB(0xF2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/**************************   packed generic (SSE2)   *************************/

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

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#if (RT_128 < 4)

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

#else /* RT_128 >= 4 */

#define mmvjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x38) EMITB(0x15)           \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x38) EMITB(0x15)           \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_128 >= 4 */

#define mmvjx_st(XS, MG, DG)                                                \
        andjx_rr(W(XS), Xmm0)                                               \
        annjx_ld(Xmm0, W(MG), W(DG))                                        \
        orrjx_rr(Xmm0, W(XS))                                               \
        movjx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S) */

#define andjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* ann (G = ~G & S) */

#define annjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orr (G = G | S) */

#define orrjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orn (G = ~G | S) */

#define ornjx_rr(XG, XS)                                                    \
        notjx_rx(W(XG))                                                     \
        orrjx_rr(W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        notjx_rx(W(XG))                                                     \
        orrjx_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* not (G = ~G) */

#define notjx_rx(XG)                                                        \
        annjx_ld(W(XG), Mebp, inf_GPC07)

/**************   packed double precision floating point (SSE2)   *************/

/* neg (G = -G) */

#define negjs_rx(XG)                                                        \
        xorjx_ld(W(XG), Mebp, inf_GPC06_64)

/* add (G = G + S), (D = S + T) */

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

/* sub (G = G - S), (D = S - T) */

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

/* mul (G = G * S), (D = S * T) */

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

/* div (G = G / S), (D = S / T) if (D != S) */

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

        /* cbe, cbs, cbr defined in rtbase.h
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

        /* rcp defined in rtbase.h
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

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T)
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

/* fma (G = G + S * T)
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

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
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

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
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

/* min (G = G < S ? G : S) */

#define minjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* max (G = G > S ? G : S) */

#define maxjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cmp (G = G ? S) */

#define ceqjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cnejs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cnejs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cltjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define clejs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clejs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cgtjs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtjs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgejs_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgejs_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_128 < 4)

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
        cvzjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#else /* RT_128 >= 4 */

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#endif /* RT_128 >= 4 */

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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_128 < 4)

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

#else /* RT_128 >= 4 */

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

#endif /* RT_128 >= 4 */

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_128 < 4)

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

#else /* RT_128 >= 4 */

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

#endif /* RT_128 >= 4 */

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_128 < 4)

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
        cvnjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
        cvnjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#else /* RT_128 >= 4 */

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
ADR ESC REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#endif /* RT_128 >= 4 */

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

/**************************   packed integer (SSE2)   *************************/

/* add (G = G + S) */

#define addjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD4)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub (G = G - S) */

#define subjx_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subjx_ld(XG, MS, DS)                                                \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xFB)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shljx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xF3)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrjx_ri(XG, IS)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x73)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
ADR ESC REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xD3)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))


#define shrjn_ri(XG, IS)                                                    \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DS))                                        \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   helper macros (SSE2)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE64_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_128    0x0F     /*  all satisfy the condition */

#define mkjjx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        REX(0,       RXB(XS)) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##64_128))                     \
        jeqxx_lb(lb)

#if (RT_128 < 4)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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

#else /* RT_128 >= 4 */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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

#endif /* RT_128 >= 4 */

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#if (RT_128 < 4)

#define rnrjs_rr(XD, XS, mode)                                              \
        cvrjs_rr(W(XD), W(XS), mode)                                        \
        cvnjn_rr(W(XD), W(XD))

#define cvrjs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_128 >= 4 */

#define rnrjs_rr(XD, XS, mode)                                              \
    ESC REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x09)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrjs_rr(XD, XS, mode)                                              \
        rnrjs_rr(W(XD), W(XS), mode)                                        \
        cvzjs_rr(W(XD), W(XD))

#endif /* RT_128 >= 4 */

/**************   scalar double precision floating point (SSE2)   *************/

/* mov (D = S) */

#define movtx_rr(XD, XS)                                                    \
    xF2 REX(RXB(XD), RXB(XS)) EMITB(0x0F) EMITB(0x10)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movtx_ld(XD, MS, DS)                                                \
ADR xF2 REX(RXB(XD), RXB(MS)) EMITB(0x0F) EMITB(0x10)                       \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movtx_st(XS, MD, DD)                                                \
ADR xF2 REX(RXB(XS), RXB(MD)) EMITB(0x0F) EMITB(0x11)                       \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S) */

#define addts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub (G = G - S) */

#define subts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* mul (G = G * S) */

#define mults_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mults_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* div (G = G / S) */

#define divts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

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
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR01(0))

#define rcsts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsets_rr(XD, XS)                                                    \
        sqrts_rr(W(XD), W(XS))                                              \
        movtx_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movtx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR01(0))

#define rssts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmats_rr(XG, XS, XT)                                                \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_rr(W(XS), W(XT))                                              \
        addts_rr(W(XG), W(XS))                                              \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmats_ld(XG, XS, MT, DT)                                            \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_ld(W(XS), W(MT), W(DT))                                       \
        addts_rr(W(XG), W(XS))                                              \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmats_rr(XG, XS, XT)                                                \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmats_rx(W(XG))

#define fmats_ld(XG, XS, MT, DT)                                            \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_ld(W(XS), W(MT), W(DT))                                       \
        movtx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmats_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmats_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmats_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmats_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_ld(W(XS), W(MT), W(DT))                                       \
        movtx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmats_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmats_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        movtx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movtx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsts_rr(XG, XS, XT)                                                \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_rr(W(XS), W(XT))                                              \
        subts_rr(W(XG), W(XS))                                              \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mults_ld(W(XS), W(MT), W(DT))                                       \
        subts_rr(W(XG), W(XS))                                              \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsts_rr(XG, XS, XT)                                                \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsts_rx(W(XG))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_ld(W(XS), W(MT), W(DT))                                       \
        movtx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsts_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsts_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsts_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movtx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movtx_ld(W(XS), W(MT), W(DT))                                       \
        movtx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsts_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsts_rx(XG) /* not portable, do not use outside */                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        movtx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrzs_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR02(0x00))                                    \
        movtx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define mints_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mints_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* max (G = G > S ? G : S) */

#define maxts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cmp (G = G ? S) */

#define ceqts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cnets_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cnets_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cltts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define clets_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clets_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cgtts_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtts_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgets_rr(XG, XS)                                                    \
    xF2 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgets_ld(XG, MS, DS)                                                \
ADR xF2 REX(RXB(XG), RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

/**************************   extended float (x87)   **************************/

#define fpuzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDD)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuzs_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDD)                                   \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpuzn_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDF)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuzn_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDF)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#define fpuzt_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDD)                                   \
        MRM(0x01,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define addzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbrzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x01,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define dvrzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define fpucw_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD9)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpucw_st(MD, DD) /* not portable, do not use outside */             \
FWT ADR REX(0,       RXB(MD)) EMITB(0xD9)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpurz_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR02(4))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_128V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

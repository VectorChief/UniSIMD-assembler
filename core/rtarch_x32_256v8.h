/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_256V8_H
#define RT_RTARCH_X32_256V8_H

#include "rtarch_x64.h"

#define RT_SIMD_REGS_256        8
#define RT_SIMD_ALIGN_256       32
#define RT_SIMD_WIDTH64_256     4
#define RT_SIMD_SET64_256(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH32_256     8
#define RT_SIMD_SET32_256(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_256v8.h: Implementation of x86_64 fp32 SSE2/4 instruction pairs.
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

#if defined (RT_256) && (RT_256 >= 8)

#ifndef RT_RTARCH_X64_128V4_H
#undef  RT_128
#define RT_128  (RT_SIMD_COMPAT_256)
#include "rtarch_x64_128v4.h"
#endif /* RT_RTARCH_X64_128V4_H */

#undef  sregs_sa
#undef  sregs_la

/* mandatory escape prefix for some opcodes (must preceed rex) */
#define ESC                                                                 \
        EMITB(0x66)

/* mandatory escape prefix for some opcodes (must preceed rex) */
#define xF3                                                                 \
        EMITB(0xF3)

/* fwait instruction for legacy processors (fix for fstcw) */
#define FWT                                                                 \
        EMITB(0x9B)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/**************************   packed generic (SSE1)   *************************/

/* mov (D = S) */

#define movcx_rr(XD, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movcx_ld(XD, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define movcx_st(XS, MD, DD)                                                \
    ADR REX(0,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR REX(1,       RXB(MD)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VYL(DD)), EMPTY)

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvcx_rr(XG, XS)                                                    \
        andcx_rr(W(XS), Xmm0)                                               \
        anncx_rr(Xmm0, W(XG))                                               \
        orrcx_rr(Xmm0, W(XS))                                               \
        movcx_rr(W(XG), Xmm0)

#define mmvcx_ld(XG, MS, DS)                                                \
        notcx_rx(Xmm0)                                                      \
        andcx_rr(W(XG), Xmm0)                                               \
        anncx_ld(Xmm0, W(MS), W(DS))                                        \
        orrcx_rr(W(XG), Xmm0)

#define mmvcx_st(XS, MG, DG)                                                \
        andcx_rr(W(XS), Xmm0)                                               \
        anncx_ld(Xmm0, W(MG), W(DG))                                        \
        orrcx_rr(Xmm0, W(XS))                                               \
        movcx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S) */

#define andcx_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andcx_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* ann (G = ~G & S) */

#define anncx_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define anncx_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* orr (G = G | S) */

#define orrcx_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrcx_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* orn (G = ~G | S) */

#define orncx_rr(XG, XS)                                                    \
        notcx_rx(W(XG))                                                     \
        orrcx_rr(W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        notcx_rx(W(XG))                                                     \
        orrcx_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorcx_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorcx_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* not (G = ~G) */

#define notcx_rx(XG)                                                        \
        anncx_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (SSE1)   *************/

/* neg (G = -G) */

#define negcs_rx(XG)                                                        \
        xorcx_ld(W(XG), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) */

#define addcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define addcs3rr(XD, XS, XT)                                                \
        movcx_rr(W(XD), W(XS))                                              \
        addcs_rr(W(XD), W(XT))

#define addcs3ld(XD, XS, MT, DT)                                            \
        movcx_rr(W(XD), W(XS))                                              \
        addcs_ld(W(XD), W(MT), W(DT))

/* sub (G = G - S), (D = S - T) */

#define subcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define subcs3rr(XD, XS, XT)                                                \
        movcx_rr(W(XD), W(XS))                                              \
        subcs_rr(W(XD), W(XT))

#define subcs3ld(XD, XS, MT, DT)                                            \
        movcx_rr(W(XD), W(XS))                                              \
        subcs_ld(W(XD), W(MT), W(DT))

/* mul (G = G * S), (D = S * T) */

#define mulcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define mulcs3rr(XD, XS, XT)                                                \
        movcx_rr(W(XD), W(XS))                                              \
        mulcs_rr(W(XD), W(XT))

#define mulcs3ld(XD, XS, MT, DT)                                            \
        movcx_rr(W(XD), W(XS))                                              \
        mulcs_ld(W(XD), W(MT), W(DT))

/* div (G = G / S), (D = S / T) if (D != S) */

#define divcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#define divcs3rr(XD, XS, XT)                                                \
        movcx_rr(W(XD), W(XS))                                              \
        divcs_rr(W(XD), W(XT))

#define divcs3ld(XD, XS, MT, DT)                                            \
        movcx_rr(W(XD), W(XS))                                              \
        divcs_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrcs_ld(XD, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcecs_rr(XD, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x53)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x53)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcscs_rr(XG, XS) /* destroys XS */                                  \
        mulcs_rr(W(XS), W(XG))                                              \
        mulcs_rr(W(XS), W(XG))                                              \
        addcs_rr(W(XG), W(XG))                                              \
        subcs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsecs_rr(XD, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x52)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x52)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rsscs_rr(XG, XS) /* destroys XS */                                  \
        mulcs_rr(W(XS), W(XG))                                              \
        mulcs_rr(W(XS), W(XG))                                              \
        subcs_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulcs_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulcs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmacs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_rr(W(XS), W(XT))                                              \
        addcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_ld(W(XS), W(MT), W(DT))                                       \
        addcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmacs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmacs_rx(W(XG))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XS), W(MT), W(DT))                                       \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmacs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmacs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmacs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XS), W(MT), W(DT))                                       \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmacs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmacs_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x14))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x14))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x1C))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x1C))                                    \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addws_ld(Mebp,  inf_SCR02(0x1C))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x1C))                                    \
        addws_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x18))                                    \
        addws_ld(Mebp,  inf_SCR02(0x14))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x14))                                    \
        addws_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x10))                                    \
        addws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x0C))                                    \
        addws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x08))                                    \
        addws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x04))                                    \
        addws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movcx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmscs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_rr(W(XS), W(XT))                                              \
        subcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulcs_ld(W(XS), W(MT), W(DT))                                       \
        subcs_rr(W(XG), W(XS))                                              \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmscs_rr(XG, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmscs_rx(W(XG))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XS), W(MT), W(DT))                                       \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmscs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmscs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmscs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XS), W(MT), W(DT))                                       \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movcx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmscs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmscs_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x10))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x14))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x14))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x18))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x1C))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x1C))                                    \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrws_ld(Mebp,  inf_SCR02(0x1C))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x1C))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x18))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x18))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x14))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x14))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x10))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x10))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x0C))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x08))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x04))                                    \
        sbrws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movcx_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define mincs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mincs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* max (G = G > S ? G : S) */

#define maxcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* cmp (G = G ? S) */

#define ceqcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x00))

#define cnecs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cnecs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x04))

#define cltcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x01))

#define clecs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clecs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x02))

#define cgtcs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtcs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x06))                           \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x06))

#define cgecs_rr(XG, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        REX(1,             1) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgecs_ld(XG, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x05))                           \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x05))

#if (RT_SIMD_COMPAT_256 < 2)

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        cvzcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x0C))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x14))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x14))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x18))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x1C))                                    \
        fpuwt_st(Mebp,  inf_SCR01(0x1C))                                    \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        cvpcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvncs_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvncs_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        cvmcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvncs_rr(W(XD), W(XS))                                              \
        fpurn_xx()

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvncs_ld(W(XD), W(MS), W(DS))                                       \
        fpurn_xx()

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        cvncs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        cvncs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x0C))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x14))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x14))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x18))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x1C))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x1C))                                    \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        cvncs_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuwn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x0C))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x14))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x14))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x18))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x1C))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x1C))                                    \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_256 >= 2 */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_SIMD_COMPAT_256 < 4)

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        cvzcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_256 >= 4 */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x03))

#endif /* RT_SIMD_COMPAT_256 >= 4 */

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
    xF3 REX(0,             0) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
    xF3 REX(1,             1) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
ADR xF3 REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR xF3 REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_SIMD_COMPAT_256 < 4)

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        cvpcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        cvpcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#else /* RT_SIMD_COMPAT_256 >= 4 */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x02))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        rnpcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_256 >= 4 */

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_SIMD_COMPAT_256 < 4)

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        cvmcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        cvmcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#else /* RT_SIMD_COMPAT_256 >= 4 */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x01))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        rnmcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_256 >= 4 */

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_SIMD_COMPAT_256 < 4)

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        cvncs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        cvncs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_256 >= 4 */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x00))

#endif /* RT_SIMD_COMPAT_256 >= 4 */

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        cvtcs_rr(W(XD), W(XS))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        cvtcs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        cvtcn_rr(W(XD), W(XS))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        cvtcn_ld(W(XD), W(MS), W(DS))

#endif /* RT_SIMD_COMPAT_256 >= 2 */

/**************************   packed integer (SSE2)   *************************/

/* add (G = G + S) */

#define addcx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xFE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xFE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addcx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xFE)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xFE)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* sub (G = G - S) */

#define subcx_rr(XG, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0xFA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0xFA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subcx_ld(XG, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xFA)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xFA)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlcx_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xF2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xF2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrcx_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xD2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xD2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)


#define shrcn_ri(XG, IS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
    ESC REX(0,             1) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0xE2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0xE2)                       \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
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
        stack_ld(Recx)                                                      \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XG), W(MS), W(DS))                                       \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
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
        stack_ld(Recx)                                                      \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
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
        stack_ld(Recx)                                                      \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XG), W(MS), W(DS))                                       \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
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
        stack_ld(Recx)                                                      \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))


#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XS), Mebp, inf_SCR02(0))                                 \
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
        stack_ld(Recx)                                                      \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XG), W(MS), W(DS))                                       \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
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
        stack_ld(Recx)                                                      \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   helper macros (SSE1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE32_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256    0x0F     /*  all satisfy the condition */

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        REX(0,             0) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(1,             0) EMITB(0x8B)                                   \
        MRM(0x07,       0x03, 0x00)                                         \
        REX(0,             1) EMITB(0x0F) EMITB(0x50)                       \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##32_256 >> 3) << 1)))   \
        MRM(0x00,       0x03, 0x07)                                         \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_256))                     \
        jeqxx_lb(lb)

#if (RT_SIMD_COMPAT_256 < 2)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        cvtcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rndcs_ld(XD, MS, DS)                                                \
        cvtcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvtcs_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvncs_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtcs_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        cvtcs_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtcn_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvncn_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtcn_ld(XD, MS, DS)                                                \
        movcx_ld(W(XD), W(MS), W(DS))                                       \
        cvtcn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_256 >= 2 */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_SIMD_COMPAT_256 < 4)

#define rndcs_rr(XD, XS)                                                    \
        cvtcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rndcs_ld(XD, MS, DS)                                                \
        cvtcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_256 >= 4 */

#define rndcs_rr(XD, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndcs_ld(XD, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMITB(0x04))

#endif /* RT_SIMD_COMPAT_256 >= 4 */

#define cvtcs_rr(XD, XS)                                                    \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcs_ld(XD, MS, DS)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
ADR ESC REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtcn_rr(XD, XS)                                                    \
        REX(0,             0) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        REX(1,             1) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcn_ld(XD, MS, DS)                                                \
    ADR REX(0,       RXB(MS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR REX(1,       RXB(MS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VYL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_256 >= 2 */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_SIMD_COMPAT_256 < 4)

#define rnrcs_rr(XD, XS, mode)                                              \
        cvrcs_rr(W(XD), W(XS), mode)                                        \
        cvncn_rr(W(XD), W(XD))

#define cvrcs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_SIMD_COMPAT_256 >= 4 */

#define rnrcs_rr(XD, XS, mode)                                              \
    ESC REX(0,             0) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
    ESC REX(1,             1) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrcs_rr(XD, XS, mode)                                              \
        rnrcs_rr(W(XD), W(XS), mode)                                        \
        cvzcs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_256 >= 4 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movcx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm7, Oeax, PLAIN)

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movcx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm7, Oeax, PLAIN)

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_256V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

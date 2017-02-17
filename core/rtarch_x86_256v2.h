/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_256V2_H
#define RT_RTARCH_X86_256V2_H

#include "rtarch_x86.h"

#define RT_SIMD_REGS_256        16
#define RT_SIMD_ALIGN_256       32
#define RT_SIMD_WIDTH64_256     4
#define RT_SIMD_SET64_256(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH32_256     8
#define RT_SIMD_SET32_256(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86_256v2.h: Implementation of x86:i386 fp32 AVX1/2 instructions.
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

#if defined (RT_256) && (RT_256 != 0)

#ifndef RT_RTARCH_X86_128V8_H
#undef  RT_SIMD_COMPAT_128
#define RT_SIMD_COMPAT_128  (RT_256)
#include "rtarch_x86_128v8.h"
#endif /* RT_RTARCH_X86_128V8_H */

#undef  sregs_sa
#undef  sregs_la

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov (D = S) */

#define movcx_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movcx_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movcx_st(XS, MD, DD)                                                \
        V2X(0x00,    1, 0) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvcx_rr(XG, XS)                                                    \
        VEX(REN(XG), 1, 1, 3) EMITB(0x4A)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define mmvcx_ld(XG, MS, DS)                                                \
        VEX(REN(XG), 1, 1, 3) EMITB(0x4A)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define mmvcx_st(XS, MG, DG)                                                \
        VEX(0x00,    1, 1, 2) EMITB(0x2E)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* and (G = G & S), (D = S & T) if (D != S) */

#define andcx_rr(XG, XS)                                                    \
        andcx3rr(W(XG), W(XG), W(XS))

#define andcx_ld(XG, MS, DS)                                                \
        andcx3ld(W(XG), W(XG), W(MS), W(DS))

#define andcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (D != S) */

#define anncx_rr(XG, XS)                                                    \
        anncx3rr(W(XG), W(XG), W(XS))

#define anncx_ld(XG, MS, DS)                                                \
        anncx3ld(W(XG), W(XG), W(MS), W(DS))

#define anncx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anncx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (D != S) */

#define orrcx_rr(XG, XS)                                                    \
        orrcx3rr(W(XG), W(XG), W(XS))

#define orrcx_ld(XG, MS, DS)                                                \
        orrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (D != S) */

#define orncx_rr(XG, XS)                                                    \
        notcx_rx(W(XG))                                                     \
        orrcx_rr(W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        notcx_rx(W(XG))                                                     \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orncx3rr(XD, XS, XT)                                                \
        movcx_rr(W(XD), W(XS))                                              \
        orncx_rr(W(XD), W(XT))

#define orncx3ld(XD, XS, MT, DT)                                            \
        movcx_rr(W(XD), W(XS))                                              \
        orncx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (D != S) */

#define xorcx_rr(XG, XS)                                                    \
        xorcx3rr(W(XG), W(XG), W(XS))

#define xorcx_ld(XG, MS, DS)                                                \
        xorcx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorcx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorcx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G) */

#define notcx_rx(XG)                                                        \
        anncx_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (AVX1)   *************/

/* neg (G = -G) */

#define negcs_rx(XG)                                                        \
        xorcx_ld(W(XG), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (D != S) */

#define addcs_rr(XG, XS)                                                    \
        addcs3rr(W(XG), W(XG), W(XS))

#define addcs_ld(XG, MS, DS)                                                \
        addcs3ld(W(XG), W(XG), W(MS), W(DS))

#define addcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subcs_rr(XG, XS)                                                    \
        subcs3rr(W(XG), W(XG), W(XS))

#define subcs_ld(XG, MS, DS)                                                \
        subcs3ld(W(XG), W(XG), W(MS), W(DS))

#define subcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (D != S) */

#define mulcs_rr(XG, XS)                                                    \
        mulcs3rr(W(XG), W(XG), W(XS))

#define mulcs_ld(XG, MS, DS)                                                \
        mulcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* div (G = G / S), (D = S / T) if (D != S) */

#define divcs_rr(XG, XS)                                                    \
        divcs3rr(W(XG), W(XG), W(XS))

#define divcs_ld(XG, MS, DS)                                                \
        divcs3ld(W(XG), W(XG), W(MS), W(DS))

#define divcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrcs_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 0) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcecs_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x53)                                      \
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
        V2X(0x00,    1, 0) EMITB(0x52)                                      \
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

#if (RT_256 < 2)

#define addwm_ri(MG, IS)     /* not portable, do not use outside */         \
        EMITB(0x81 | TYP(IS))                                               \
        MRM(0x00,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subwm_ri(MG, IS)     /* not portable, do not use outside */         \
        EMITB(0x81 | TYP(IS))                                               \
        MRM(0x05,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))


#define cvycs_rr(XD, XS)     /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x5A)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvycs_ld(XD, MS, DS) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x5A)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cvxds_rr(XD, XS)     /* not portable, do not use outside */         \
        V2X(0x00,    1, 1) EMITB(0x5A)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))


#define addds_ld(XG, MS, DS) /* not portable, do not use outside */         \
        V2X(REG(XG), 1, 1) EMITB(0x58)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subds_ld(XG, MS, DS) /* not portable, do not use outside */         \
        V2X(REG(XG), 1, 1) EMITB(0x5C)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulds_ld(XG, MS, DS) /* not portable, do not use outside */         \
        V2X(REG(XG), 1, 1) EMITB(0x59)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

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

#define fmacs_rr(XG, XS, XT)                                                \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvycs_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        addwm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        subwm_ri(W(MT), IC(0x10))                  /* 2st-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

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

#define fmscs_rr(XG, XS, XT)                                                \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvycs_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        prmcx_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        addwm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        subwm_ri(W(MT), IC(0x10))                  /* 2st-pass <- */        \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_256 >= 2 */ /* FMA comes with AVX2 */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 1, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 1, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_256 >= 2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (D != S) */

#define mincs_rr(XG, XS)                                                    \
        mincs3rr(W(XG), W(XG), W(XS))

#define mincs_ld(XG, MS, DS)                                                \
        mincs3ld(W(XG), W(XG), W(MS), W(DS))

#define mincs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mincs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (D != S) */

#define maxcs_rr(XG, XS)                                                    \
        maxcs3rr(W(XG), W(XG), W(XS))

#define maxcs_ld(XG, MS, DS)                                                \
        maxcs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cmp (G = G ? S), (D = S ? T) if (D != S) */

#define ceqcs_rr(XG, XS)                                                    \
        ceqcs3rr(W(XG), W(XG), W(XS))

#define ceqcs_ld(XG, MS, DS)                                                \
        ceqcs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))


#define cnecs_rr(XG, XS)                                                    \
        cnecs3rr(W(XG), W(XG), W(XS))

#define cnecs_ld(XG, MS, DS)                                                \
        cnecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cnecs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))


#define cltcs_rr(XG, XS)                                                    \
        cltcs3rr(W(XG), W(XG), W(XS))

#define cltcs_ld(XG, MS, DS)                                                \
        cltcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))


#define clecs_rr(XG, XS)                                                    \
        clecs3rr(W(XG), W(XG), W(XS))

#define clecs_ld(XG, MS, DS)                                                \
        clecs3ld(W(XG), W(XG), W(MS), W(DS))

#define clecs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clecs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))


#define cgtcs_rr(XG, XS)                                                    \
        cgtcs3rr(W(XG), W(XG), W(XS))

#define cgtcs_ld(XG, MS, DS)                                                \
        cgtcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtcs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))


#define cgecs_rr(XG, XS)                                                    \
        cgecs3rr(W(XG), W(XG), W(XS))

#define cgecs_ld(XG, MS, DS)                                                \
        cgecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgecs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 1, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        V2X(0x00,    1, 2) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        V2X(0x00,    1, 2) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        rnpcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        rnmcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

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

/**************************   packed integer (AVX1)   *************************/

#if (RT_256 < 2)

#define prmcx_rr(XD, XS, IT) /* not portable, do not use outside */         \
        VEX(REG(XD), 1, 1, 3) EMITB(0x06)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

/* add (G = G + S) */

#define addcx_rr(XG, XS)                                                    \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        addix_rr(W(XG), W(XS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        addix_rr(W(XG), W(XS))                                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define addcx_ld(XG, MS, DS)                                                \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XG), W(MS), W(DS))                                       \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

/* sub (G = G - S) */

#define subcx_rr(XG, XS)                                                    \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        subix_rr(W(XG), W(XS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        subix_rr(W(XG), W(XS))                                              \
        prmcx_rr(W(XS), W(XS), IB(1))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define subcx_ld(XG, MS, DS)                                                \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XG), W(MS), W(DS))                                       \
        movcx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlcx_ri(XG, IS)                                                    \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

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

#define shrcx_ri(XG, IS)                                                    \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

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


#define shrcn_ri(XG, IS)                                                    \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movcx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movcx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
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

/**************************   packed integer (AVX2)   *************************/

#else /* RT_256 >= 2 */

/* add (G = G + S) */

#define addcx_rr(XG, XS)                                                    \
        V2X(REG(XG), 1, 1) EMITB(0xFE)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addcx_ld(XG, MS, DS)                                                \
        V2X(REG(XG), 1, 1) EMITB(0xFE)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub (G = G - S) */

#define subcx_rr(XG, XS)                                                    \
        V2X(REG(XG), 1, 1) EMITB(0xFA)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subcx_ld(XG, MS, DS)                                                \
        V2X(REG(XG), 1, 1) EMITB(0xFA)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlcx_ri(XG, IS)                                                    \
        V2X(REG(XG), 1, 1) EMITB(0x72)                                      \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        V2X(REG(XG), 1, 1) EMITB(0xF2)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(REG(XG), 1, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(REG(XG), 1, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrcx_ri(XG, IS)                                                    \
        V2X(REG(XG), 1, 1) EMITB(0x72)                                      \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        V2X(REG(XG), 1, 1) EMITB(0xD2)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(REG(XG), 1, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(REG(XG), 1, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define shrcn_ri(XG, IS)                                                    \
        V2X(REG(XG), 1, 1) EMITB(0x72)                                      \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        V2X(REG(XG), 1, 1) EMITB(0xE2)                                      \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(REG(XG), 1, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(REG(XG), 1, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_256 >= 2 */

/**************************   helper macros (AVX1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE32_256    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256    0xFF     /*  all satisfy the condition */

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        V2X(0x00,    1, 0) EMITB(0x50)                                      \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_256))                     \
        jeqxx_lb(lb)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndcs_ld(XD, MS, DS)                                                \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtcs_rr(XD, XS)                                                    \
        V2X(0x00,    1, 1) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcs_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 1) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtcn_rr(XD, XS)                                                    \
        V2X(0x00,    1, 0) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtcn_ld(XD, MS, DS)                                                \
        V2X(0x00,    1, 0) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        VEX(0x00,    1, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvrcs_rr(XD, XS, mode)                                              \
        rnrcs_rr(W(XD), W(XS), mode)                                        \
        cvzcs_rr(W(XD), W(XD))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#if   (RT_SIMD == 256)

#define muvcx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define muvcx_st(XS, MD, DD) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#elif (RT_SIMD == 128)

#define muvcx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x10)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define muvcx_st(XS, MD, DD) /* not portable, do not use outside */         \
        V2X(0x00,    1, 0) EMITB(0x11)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#endif /* RT_SIMD: 256, 128 */

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        muvcx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_st(Xmm7, Oeax, PLAIN)

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        muvcx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        muvcx_ld(Xmm7, Oeax, PLAIN)

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_256V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

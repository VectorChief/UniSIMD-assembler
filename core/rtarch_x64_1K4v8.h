/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_1K4V8_H
#define RT_RTARCH_X64_1K4V8_H

#include "rtarch_x32_1K4v8.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64_1K4v8.h: Implementation of x86_64 fp64 AVX-512 instruction pairs.
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

#if defined (RT_1K4) && (RT_1K4 >= 8)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX3)   *************************/

/* mov (D = S) */

#define movqx_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movqx_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#define movqx_st(XS, MD, DD)                                                \
    ADR EVW(RXB(XS), RXB(MD),    0x00, K, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR EVW(RMB(XS), RXB(MD),    0x00, K, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VZL(DD)), EMPTY)

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvqx_rr(XG, XS)                                                    \
        ck1qx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(RXB(XG), RXB(XS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1qx_rm(XmmG, Mebp, inf_GPC07)                                     \
        EKW(RMB(XG), RMB(XS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvqx_ld(XG, MS, DS)                                                \
        ck1qx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XG), RXB(MS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        ck1qx_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKW(RMB(XG), RXB(MS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#define mmvqx_st(XS, MG, DG)                                                \
        ck1qx_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKW(RXB(XS), RXB(MG),    0x00, K, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VAL(DG)), EMPTY)                                 \
        ck1qx_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKW(RMB(XS), RXB(MG),    0x00, K, 1, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VZL(DG)), EMPTY)

#define ck1qx_rm(XS, MT, DT) /* not portable, do not use outside */         \
    ADR EVW(0,       RXB(MT), REN(XS), K, 1, 2) EMITB(0x29)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

#if (RT_SIMD_COMPAT_1K4 < 2)

/* and (G = G & S) */

#define andqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* ann (G = ~G & S) */

#define annqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* orr (G = G | S) */

#define orrqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* orn (G = ~G | S) */

#define ornqx_rr(XG, XS)                                                    \
        notqx_rx(W(XG))                                                     \
        orrqx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        notqx_rx(W(XG))                                                     \
        orrqx_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

/* and (G = G & S) */

#define andqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* ann (G = ~G & S) */

#define annqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* orr (G = G | S) */

#define orrqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* orn (G = ~G | S) */

#define ornqx_rr(XG, XS)                                                    \
        notqx_rx(W(XG))                                                     \
        orrqx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        notqx_rx(W(XG))                                                     \
        orrqx_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* not (G = ~G) */

#define notqx_rx(XG)                                                        \
        annqx_ld(W(XG), Mebp, inf_GPC07)

/**************   packed double precision floating point (AVX3)   *************/

/* neg (G = -G) */

#define negqs_rx(XG)                                                        \
        xorqx_ld(W(XG), Mebp, inf_GPC06_64)

/* add (G = G + S) */

#define addqs_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addqs_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* sub (G = G - S) */

#define subqs_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subqs_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* mul (G = G * S) */

#define mulqs_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulqs_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* div (G = G / S) */

#define divqs_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divqs_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x5E)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* sqr (D = sqrt S) */

#define sqrqs_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrqs_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0

#define rceqs_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsqs_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RCP == 2

#define rceqs_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsqs_rr(XG, XS) /* destroys XS */                                  \
        mulqs_rr(W(XS), W(XG))                                              \
        mulqs_rr(W(XS), W(XG))                                              \
        addqs_rr(W(XG), W(XG))                                              \
        subqs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0

#define rseqs_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssqs_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RSQ == 2

#define rseqs_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssqs_rr(XG, XS) /* destroys XS */                                  \
        mulqs_rr(W(XS), W(XG))                                              \
        mulqs_rr(W(XS), W(XG))                                              \
        subqs_ld(W(XS), Mebp, inf_GPC03_64)                                 \
        mulqs_ld(W(XS), Mebp, inf_GPC02_64)                                 \
        mulqs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaqs_rr(XG, XS, XT)                                                \
    ADR EVW(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVW(RMB(XG), RMB(XT), REM(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
    ADR EVW(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MT), REM(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsqs_rr(XG, XS, XT)                                                \
    ADR EVW(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVW(RMB(XG), RMB(XT), REM(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
    ADR EVW(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MT), REM(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minqs_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minqs_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x5D)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* max (G = G > S ? G : S) */

#define maxqs_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxqs_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0x5F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* cmp (G = G ? S) */

#define ceqqs_rr(XG, XS)                                                    \
        EVW(0,       RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVW(0,       RMB(XS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define ceqqs_ld(XG, MS, DS)                                                \
    ADR EVW(0,       RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x00))                           \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cneqs_rr(XG, XS)                                                    \
        EVW(0,       RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVW(0,       RMB(XS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cneqs_ld(XG, MS, DS)                                                \
    ADR EVW(0,       RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x04))                           \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cltqs_rr(XG, XS)                                                    \
        EVW(0,       RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVW(0,       RMB(XS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cltqs_ld(XG, MS, DS)                                                \
    ADR EVW(0,       RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x01))                           \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cleqs_rr(XG, XS)                                                    \
        EVW(0,       RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVW(0,       RMB(XS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cleqs_ld(XG, MS, DS)                                                \
    ADR EVW(0,       RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x02))                           \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cgtqs_rr(XG, XS)                                                    \
        EVW(0,       RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVW(0,       RMB(XS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cgtqs_ld(XG, MS, DS)                                                \
    ADR EVW(0,       RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x06))                           \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x06))                           \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cgeqs_rr(XG, XS)                                                    \
        EVW(0,       RXB(XS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVW(0,       RMB(XS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define cgeqs_ld(XG, MS, DS)                                                \
    ADR EVW(0,       RXB(MS), REN(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x05))                           \
        mz1qx_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVW(0,       RXB(MS), REM(XG), K, 1, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x05))                           \
        mz1qx_ld(X(XG), Mebp, inf_GPC07)

#define mz1qx_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR EZW(RXB(XG), RXB(MS),    0x00, K, 1, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzqs_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x03))

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x18))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x20))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x20))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x28))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x28))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x30))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x30))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x38))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x38))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x40))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x40))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x48))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x48))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x50))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x50))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x58))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x58))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x60))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x60))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x68))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x68))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x70))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x70))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x78))                                    \
        fpuzt_st(Mebp,  inf_SCR01(0x78))                                    \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x7A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 1) EMITB(0x7A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x7A)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x7A)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpqs_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x02))

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        rnpqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        ERW(RXB(XD), RXB(XS),    0x00, 2, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERW(RMB(XD), RMB(XS),    0x00, 2, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        movqs_ld(W(XD), W(MS), W(DS))                                       \
        cvpqs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XD, XS)     /* round towards -inf */                       \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmqs_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x01))

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        rnmqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        ERW(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERW(RMB(XD), RMB(XS),    0x00, 1, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        movqs_ld(W(XD), W(MS), W(DS))                                       \
        cvmqs_rr(W(XD), W(XD))

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XD, XS)     /* round towards near */                       \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnqs_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x00))

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzs_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x18))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x20))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x20))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x28))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x28))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x30))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x30))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x38))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x38))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x40))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x40))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x48))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x48))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x50))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x50))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x58))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x58))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x60))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x60))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x68))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x68))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x70))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x70))                                    \
        fpuzs_ld(Mebp,  inf_SCR01(0x78))                                    \
        fpuzn_st(Mebp,  inf_SCR01(0x78))                                    \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnqs_ld(XD, MS, DS) /* round towards near */                       \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvnqs_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvnqs_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuzn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x10))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x10))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x18))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x18))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x20))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x20))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x28))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x28))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x30))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x30))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x38))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x38))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x40))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x40))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x48))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x48))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x50))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x50))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x58))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x58))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x60))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x60))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x68))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x68))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x70))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x70))                                    \
        fpuzn_ld(Mebp,  inf_SCR01(0x78))                                    \
        fpuzs_st(Mebp,  inf_SCR01(0x78))                                    \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

#define cvnqn_ld(XD, MS, DS) /* round towards near */                       \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvnqn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        EVW(RXB(XD), RXB(XS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvnqn_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/**************************   packed integer (AVX3)   *************************/

/* add (G = G + S) */

#define addqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xD4)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* sub (G = G - S) */

#define subqx_rr(XG, XS)                                                    \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subqx_ld(XG, MS, DS)                                                \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xFB)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlqx_ri(XG, IS)                                                    \
        EVW(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))                        \
        EVW(0,       RMB(XG), REM(XG), K, 1, 1) EMITB(0x73)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xF3)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrqx_ri(XG, IS)                                                    \
        EVW(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))                        \
        EVW(0,       RMB(XG), REM(XG), K, 1, 1) EMITB(0x73)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xD3)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)


#define shrqn_ri(XG, IS)                                                    \
        EVW(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))                        \
        EVW(0,       RMB(XG), REM(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EVW(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVW(RMB(XG), RMB(XS), REM(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        EVW(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVW(RMB(XG), RXB(MS), REM(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/**************************   helper macros (AVX3)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE64_1K4    0x0000   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_1K4    0xFFFF   /*  all satisfy the condition */

/* #define mk1wx_rx(RD)                    (defined in 32_1K4-bit header) */
/* #define ck1ox_rm(XS, MT, DT)            (defined in 32_1K4-bit header) */

#define mkjqx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1ox_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(1,             0) EMITB(0x8B)                                   \
        MRM(0x07,       0x03, 0x00)                                         \
        ck1ox_rm(X(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##64_1K4 >> 15) << 1)))  \
        MRM(0x00,       0x03, 0x07)                                         \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##64_1K4))                     \
        jeqxx_lb(lb)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndqs_ld(XD, MS, DS)                                                \
    ADR EVW(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR EVW(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x04))

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvtqs_rr(XD, XS)                                                    \
        rndqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvtqs_ld(XD, MS, DS)                                                \
        rndqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvtqs_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtqs_ld(XD, MS, DS)                                                \
        EVW(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVW(RMB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x7B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvtqn_rr(XD, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnqn_rr(W(XD), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtqn_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        cvtqn_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvtqn_rr(XD, XS)                                                    \
        EVW(RXB(XD), RXB(XS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVW(RMB(XD), RMB(XS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtqn_ld(XD, MS, DS)                                                \
        EVW(RXB(XD), RXB(MS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVW(RMB(XD), RXB(MS),    0x00, K, 2, 1) EMITB(0xE6)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        EVW(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVW(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x09)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#if (RT_SIMD_COMPAT_1K4 < 2)

#define cvrqs_rr(XD, XS, mode)                                              \
        rnrqs_rr(W(XD), W(XS), mode)                                        \
        cvzqs_rr(W(XD), W(XD))

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

#define cvrqs_rr(XD, XS, mode)                                              \
        ERW(RXB(XD), RXB(XS), 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x7B)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERW(RMB(XD), RMB(XS), 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x7B)\
        MRM(REG(XD), MOD(XS), REG(XS))

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_1K4 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X64_1K4V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

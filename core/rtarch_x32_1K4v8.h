/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_1K4V8_H
#define RT_RTARCH_X32_1K4V8_H

#include "rtarch_x64.h"

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       128
#define RT_SIMD_WIDTH64     16
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=v
#define RT_SIMD_WIDTH32     32
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=\
                            s[16]=s[17]=s[18]=s[19]=s[20]=s[21]=s[22]=s[23]=\
                            s[24]=s[25]=s[26]=s[27]=s[28]=s[29]=s[30]=s[31]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_1K4v8.h: Implementation of x86_64 fp32 AVX-512 instruction pairs.
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

#ifndef RT_RTARCH_X64_256V2_H
#undef  RT_256
#define RT_256  2
#include "rtarch_x64_256v2.h"
#endif /* RT_RTARCH_X64_256V2_H */

#undef  sregs_sa
#undef  sregs_la

#undef  K
#define K 2

#define XmmG    0x10, 0x03, EMPTY

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX3)   *************************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movox_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#define movox_st(XS, MD, DD)                                                \
    ADR EVX(RXB(XS), RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR EVX(RMB(XS), RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VZL(DD)), EMPTY)

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvox_rr(XG, XS)                                                    \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(RXB(XG), RXB(XS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        ck1ox_rm(XmmG, Mebp, inf_GPC07)                                     \
        EKX(RMB(XG), RMB(XS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvox_ld(XG, MS, DS)                                                \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(RXB(XG), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        ck1ox_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKX(RMB(XG), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

#define mmvox_st(XS, MG, DG)                                                \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
    ADR EKX(RXB(XS), RXB(MG),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VAL(DG)), EMPTY)                                 \
        ck1ox_rm(XmmG, Mebp, inf_GPC07)                                     \
    ADR EKX(RMB(XS), RXB(MG),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VZL(DG)), EMPTY)

#if (RT_SIMD_COMPAT_1K4 < 2)

/* and (G = G & S), (D = S & T) if (D != S) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xDB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (D != S) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xDF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (D != S) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xEB)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (D != S) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        ornox_rr(W(XD), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        ornox_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (D != S) */

#define xorox_rr(XG, XS)                                                    \
        xorox3rr(W(XG), W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorox3ld(W(XG), W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 1, 1) EMITB(0xEF)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

#else /* RT_SIMD_COMPAT_1K4 >= 2 */

/* and (G = G & S), (D = S & T) if (D != S) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (D != S) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* orr (G = G | S), (D = S | T) if (D != S) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (D != S) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        ornox_rr(W(XD), W(XT))

#define ornox3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        ornox_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (D != S) */

#define xorox_rr(XG, XS)                                                    \
        xorox3rr(W(XG), W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorox3ld(W(XG), W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_1K4 >= 2 */

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        annox_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (AVX3)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        xorox_ld(W(XG), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (D != S) */

#define addos_rr(XG, XS)                                                    \
        addos3rr(W(XG), W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addos3ld(W(XG), W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subos_rr(XG, XS)                                                    \
        subos3rr(W(XG), W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subos3ld(W(XG), W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (D != S) */

#define mulos_rr(XG, XS)                                                    \
        mulos3rr(W(XG), W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulos3ld(W(XG), W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* div (G = G / S), (D = S / T) if (D != S) */

#define divos_rr(XG, XS)                                                    \
        divos3rr(W(XG), W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divos3ld(W(XG), W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        EVX(RXB(XD), RXB(XT), REN(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        EVX(RMB(XD), RMB(XT), REM(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
    ADR EVX(RXB(XD), RXB(MT), REN(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MT), REM(XS), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqros_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0xCA)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RCP == 2

#define rceos_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0x4C)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        addos_rr(W(XG), W(XG))                                              \
        subos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RSQ == 0

#define rseos_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0xCC)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RSQ == 2

#define rseos_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 2) EMITB(0x4E)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        subos_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulos_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
    ADR EVX(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(RMB(XG), RMB(XT), REM(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
    ADR EVX(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MT), REM(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
    ADR EVX(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR EVX(RMB(XG), RMB(XT), REM(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
    ADR EVX(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MT), REM(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VZL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minos_rr(XG, XS)                                                    \
        EVX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVX(RMB(XG), RMB(XS), REM(XG), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minos_ld(XG, MS, DS)                                                \
    ADR EVX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MS), REM(XG), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* max (G = G > S ? G : S) */

#define maxos_rr(XG, XS)                                                    \
        EVX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVX(RMB(XG), RMB(XS), REM(XG), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxos_ld(XG, MS, DS)                                                \
    ADR EVX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MS), REM(XG), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* cmp (G = G ? S) */

#define ceqos_rr(XG, XS)                                                    \
        EVX(0,       RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVX(0,       RMB(XS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define ceqos_ld(XG, MS, DS)                                                \
    ADR EVX(0,       RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x00))                           \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cneos_rr(XG, XS)                                                    \
        EVX(0,       RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVX(0,       RMB(XS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cneos_ld(XG, MS, DS)                                                \
    ADR EVX(0,       RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x04))                           \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cltos_rr(XG, XS)                                                    \
        EVX(0,       RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVX(0,       RMB(XS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cltos_ld(XG, MS, DS)                                                \
    ADR EVX(0,       RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x01))                           \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cleos_rr(XG, XS)                                                    \
        EVX(0,       RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVX(0,       RMB(XS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cleos_ld(XG, MS, DS)                                                \
    ADR EVX(0,       RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x02))                           \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cgtos_rr(XG, XS)                                                    \
        EVX(0,       RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVX(0,       RMB(XS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cgtos_ld(XG, MS, DS)                                                \
    ADR EVX(0,       RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x06))                           \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x06))                           \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cgeos_rr(XG, XS)                                                    \
        EVX(0,       RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
        EVX(0,       RMB(XS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define cgeos_ld(XG, MS, DS)                                                \
    ADR EVX(0,       RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x05))                           \
        mz1ox_ld(W(XG), Mebp, inf_GPC07)                                    \
    ADR EVX(0,       RXB(MS), REM(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(0x01,       0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x05))                           \
        mz1ox_ld(X(XG), Mebp, inf_GPC07)

#define mz1ox_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR EZX(RXB(XG), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x03))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(RXB(XD), RXB(XS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x02))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        ERX(RXB(XD), RXB(XS),    0x00, 2, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(RMB(XD), RMB(XS),    0x00, 2, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cvpos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x01))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        ERX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(RMB(XD), RMB(XS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cvmos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x00))

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

/**************************   packed integer (AVX3)   *************************/

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        EVX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVX(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_ld(XG, MS, DS)                                                \
    ADR EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        EVX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVX(RMB(XG), RMB(XS), REM(XG), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_ld(XG, MS, DS)                                                \
    ADR EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        EVX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
        EVX(0,       RMB(XG), REM(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        EVX(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVX(RMB(XG), RMB(XS), REM(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)                                                    \
        EVX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
        EVX(0,       RMB(XG), REM(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        EVX(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVX(RMB(XG), RMB(XS), REM(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)


#define shron_ri(XG, IS)                                                    \
        EVX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
        EVX(0,       RMB(XG), REM(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        EVX(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        EVX(RMB(XG), RMB(XS), REM(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        EVX(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        EVX(RMB(XG), RXB(MS), REM(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/**************************   helper macros (AVX3)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE32_1K4    0x0000   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_1K4    0xFFFF   /*  all satisfy the condition */

#define mk1wx_rx(RD)         /* not portable, do not use outside */         \
        VEX(RXB(RD),       0,    0x00, 0, 0, 1) EMITB(0x93)                 \
        MRM(REG(RD),    0x03,    0x01)

#define ck1ox_rm(XS, MT, DT) /* not portable, do not use outside */         \
    ADR EVX(0,       RXB(MT), REN(XS), K, 1, 1) EMITB(0x76)                 \
        MRM(0x01,       0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1ox_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(1,             0) EMITB(0x8B)                                   \
        MRM(0x07,       0x03, 0x00)                                         \
        ck1ox_rm(X(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##32_1K4 >> 15) << 1)))  \
        MRM(0x00,       0x03, 0x07)                                         \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_1K4))                     \
        jeqxx_lb(lb)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMITB(0x04))

#define cvtos_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtos_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        EVX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        EVX(RMB(XD), RMB(XS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvton_ld(XD, MS, DS)                                                \
    ADR EVX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR EVX(RMB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VZL(DS)), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EVX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        EVX(RMB(XD), RMB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XD, XS, mode)                                              \
        ERX(RXB(XD), RXB(XS), 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)\
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        ERX(RMB(XD), RMB(XS), 0x00, RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)\
        MRM(REG(XD), MOD(XS), REG(XS))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

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
        movox_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
    ADR EVX(1,             0,    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(0x06,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*2))                               \
    ADR EVX(3,             0,    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(0x06,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*2))                               \
    ADR EVX(1,             0,    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(0x07,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*2))                               \
    ADR EVX(3,             0,    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(0x07,       0x00,    0x00)

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
        movox_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
    ADR EVX(1,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(0x06,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*2))                               \
    ADR EVX(3,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(0x06,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*2))                               \
    ADR EVX(1,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(0x07,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*2))                               \
    ADR EVX(3,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(0x07,       0x00,    0x00)

#endif /* RT_1K4 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_1K4V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

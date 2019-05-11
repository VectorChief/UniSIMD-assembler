/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_512X1V2_H
#define RT_RTARCH_X86_512X1V2_H

#include "rtarch_x86.h"

#define RT_SIMD_REGS            8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86_512x1v2.h: Implementation of x86 fp32 AVX512F/DQ ops.
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

#if (RT_512X1 >= 1 && RT_512X1 <= 2)

#ifndef RT_RTARCH_X86_256X1V2_H
#undef  RT_256X1
#define RT_256X1  2
#include "rtarch_x86_256x1v2.h"
#endif /* RT_RTARCH_X86_256X1V2_H */

#undef  K
#define K 2

/* 4-byte EVEX prefix with full customization (W0, K1, Z0) */
#define EKX(ren, len, pfx, aux)                                             \
        EMITB(0x62)                                                         \
        EMITB(0xF0 | (aux))                                                 \
        EMITB(0x00 | 1 << 2 | (0x0F - (ren)) << 3 | (pfx))                  \
        EMITB(0x09 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W1, K1, Z0) */
#define EKW(ren, len, pfx, aux)                                             \
        EMITB(0x62)                                                         \
        EMITB(0xF0 | (aux))                                                 \
        EMITB(0x80 | 1 << 2 | (0x0F - (ren)) << 3 | (pfx))                  \
        EMITB(0x09 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W0, K1, Z1) */
#define EZX(ren, len, pfx, aux)                                             \
        EMITB(0x62)                                                         \
        EMITB(0xF0 | (aux))                                                 \
        EMITB(0x00 | 1 << 2 | (0x0F - (ren)) << 3 | (pfx))                  \
        EMITB(0x89 | (len) << 5)

/* 4-byte EVEX prefix with full customization (W0, B1, RM) */
#define ERX(ren, erm, pfx, aux)                                             \
        EMITB(0x62)                                                         \
        EMITB(0xF0 | (aux))                                                 \
        EMITB(0x00 | 1 << 2 | (0x0F - (ren)) << 3 | (pfx))                  \
        EMITB(0x18 | (erm) << 5)

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
        EVX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movox_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movox_st(XS, MD, DD)                                                \
        EVX(0x00,    K, 0, 1) EMITB(0x29)                                   \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvox_rr(XG, XS)                                                    \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvox_ld(XG, MS, DS)                                                \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvox_st(XS, MG, DG)                                                \
        ck1ox_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(0x00,    K, 0, 1) EMITB(0x29)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define ck1ox_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(REG(XS), K, 1, 1) EMITB(0x76)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#if (RT_512X1 < 2)

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDB)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDB)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDF)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDF)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xEB)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xEB)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

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
        EVX(REG(XS), K, 1, 1) EMITB(0xEF)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xEF)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#else /* RT_512X1 >= 2 */

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x54)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x54)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x55)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x55)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x56)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x56)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

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
        EVX(REG(XS), K, 0, 1) EMITB(0x57)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x57)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_512X1 >= 2 */

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
        EVX(REG(XS), K, 0, 1) EMITB(0x58)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x58)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  adpos_rx
#define adpos_rx(XD) /* not portable, do not use outside */                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpis_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XD), Mebp, inf_SCR01(0x30))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subos_rr(XG, XS)                                                    \
        subos3rr(W(XG), W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subos3ld(W(XG), W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x5C)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x5C)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulos_rr(XG, XS)                                                    \
        mulos3rr(W(XG), W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulos3ld(W(XG), W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x59)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x59)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divos_rr(XG, XS)                                                    \
        divos3rr(W(XG), W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divos3ld(W(XG), W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x5E)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x5E)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EVX(0x00,    K, 0, 1) EMITB(0x51)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqros_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 0, 1) EMITB(0x51)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if   RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XD, XS)                                                    \
        EVX(0x00,    K, 1, 2) EMITB(0xCA)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RCP == 2

#define rceos_rr(XD, XS)                                                    \
        EVX(0x00,    K, 1, 2) EMITB(0x4C)                                   \
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
        EVX(0x00,    K, 1, 2) EMITB(0xCC)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys XS */

#elif RT_SIMD_COMPAT_RSQ == 2

#define rseos_rr(XD, XS)                                                    \
        EVX(0x00,    K, 1, 2) EMITB(0x4E)                                   \
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
        EVX(REG(XS), K, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minos_rr(XG, XS)                                                    \
        minos3rr(W(XG), W(XG), W(XS))

#define minos_ld(XG, MS, DS)                                                \
        minos3ld(W(XG), W(XG), W(MS), W(DS))

#define minos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x5D)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x5D)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxos_rr(XG, XS)                                                    \
        maxos3rr(W(XG), W(XG), W(XS))

#define maxos_ld(XG, MS, DS)                                                \
        maxos3ld(W(XG), W(XG), W(MS), W(DS))

#define maxos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0x5F)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0x5F)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqos_rr(XG, XS)                                                    \
        ceqos3rr(W(XG), W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqos3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define ceqos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneos_rr(XG, XS)                                                    \
        cneos3rr(W(XG), W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cneos3ld(W(XG), W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cneos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltos_rr(XG, XS)                                                    \
        cltos3rr(W(XG), W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltos3ld(W(XG), W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cltos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleos_rr(XG, XS)                                                    \
        cleos3rr(W(XG), W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        cleos3ld(W(XG), W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cleos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtos_rr(XG, XS)                                                    \
        cgtos3rr(W(XG), W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cgtos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeos_rr(XG, XS)                                                    \
        cgeos3rr(W(XG), W(XG), W(XS))

#define cgeos_ld(XG, MS, DS)                                                \
        cgeos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeos3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cgeos3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 0, 1) EMITB(0xC2)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)


#define mz1ox_ld(XG, MS, DS) /* not portable, do not use outside */         \
        EZX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_512    0x0000   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_512    0xFFFF   /*  all satisfy the condition */

#define mk1wx_rx(RD)         /* not portable, do not use outside */         \
        V2X(0x00,    0, 0) EMITB(0x93)                                      \
        MRM(REG(RD),    0x03,    0x01)

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1ox_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1wx_rx(Reax)                                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_512))                     \
        jeqxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(0x00,    K, 2, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        EVX(0x00,    K, 2, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        ERX(0x00,    2, 1, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cvpos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        ERX(0x00,    1, 1, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cvmos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

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
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtos_rr(XD, XS)                                                    \
        EVX(0x00,    K, 1, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtos_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 1, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvton_rr(XD, XS)                                                    \
        EVX(0x00,    K, 0, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvton_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 0, 1) EMITB(0x5B)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XD, XS, mode)                                              \
        ERX(0x00,    RT_SIMD_MODE_##mode&3, 1, 1) EMITB(0x5B)               \
        MRM(REG(XD), MOD(XS), REG(XS))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addox_rr(XG, XS)                                                    \
        addox3rr(W(XG), W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addox3ld(W(XG), W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xFE)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xFE)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subox_rr(XG, XS)                                                    \
        subox3rr(W(XG), W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subox3ld(W(XG), W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xFA)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xFA)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulox_rr(XG, XS)                                                    \
        mulox3rr(W(XG), W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulox3ld(W(XG), W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x40)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x40)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlox3ri(W(XG), W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlox3ld(W(XG), W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        EVX(REG(XD), K, 1, 1) EMITB(0x72)                                   \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xF2)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrox3ri(W(XG), W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrox3ld(W(XG), W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        EVX(REG(XD), K, 1, 1) EMITB(0x72)                                   \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xD2)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shron_ri(XG, IS)                                                    \
        shron3ri(W(XG), W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shron3ld(W(XG), W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        EVX(REG(XD), K, 1, 1) EMITB(0x72)                                   \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shron3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xE2)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlox3rr(W(XG), W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlox3ld(W(XG), W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrox3rr(W(XG), W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrox3ld(W(XG), W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svron3rr(W(XG), W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svron3ld(W(XG), W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movmx_rr(XD, XS)                                                    \
        EVX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movmx_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movmx_st(XS, MD, DD)                                                \
        EVX(0x00,    K, 0, 1) EMITB(0x29)                                   \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#if (RT_512X1 < 2)

#define mmvmx_rr(XG, XS)                                                    \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_rr(Xmm0, W(XG))                                               \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmx_rr(W(XG), Xmm0)

#define mmvmx_ld(XG, MS, DS)                                                \
        notmx_rx(Xmm0)                                                      \
        andmx_rr(W(XG), Xmm0)                                               \
        annmx_ld(Xmm0, W(MS), W(DS))                                        \
        orrmx_rr(W(XG), Xmm0)

#define mmvmx_st(XS, MG, DG)                                                \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_ld(Xmm0, W(MG), W(DG))                                        \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmx_st(Xmm0, W(MG), W(DG))

#else /* RT_512X1 >= 2 */

#define mmvmx_rr(XG, XS)                                                    \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(0x00,    K, 3, 1) EMITB(0x6F)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvmx_ld(XG, MS, DS)                                                \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(0x00,    K, 3, 1) EMITB(0x6F)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvmx_st(XS, MG, DG)                                                \
        ck1mx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(0x00,    K, 3, 1) EMITB(0x7F)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define ck1mx_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(REG(XS), K, 1, 1) EMITB(0x75)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_512X1 >= 2 */

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andmx_rr(XG, XS)                                                    \
        andmx3rr(W(XG), W(XG), W(XS))

#define andmx_ld(XG, MS, DS)                                                \
        andmx3ld(W(XG), W(XG), W(MS), W(DS))

#define andmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDB)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDB)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annmx_rr(XG, XS)                                                    \
        annmx3rr(W(XG), W(XG), W(XS))

#define annmx_ld(XG, MS, DS)                                                \
        annmx3ld(W(XG), W(XG), W(MS), W(DS))

#define annmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDF)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDF)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrmx_rr(XG, XS)                                                    \
        orrmx3rr(W(XG), W(XG), W(XS))

#define orrmx_ld(XG, MS, DS)                                                \
        orrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xEB)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xEB)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

#define ornmx_rr(XG, XS)                                                    \
        notmx_rx(W(XG))                                                     \
        orrmx_rr(W(XG), W(XS))

#define ornmx_ld(XG, MS, DS)                                                \
        notmx_rx(W(XG))                                                     \
        orrmx_ld(W(XG), W(MS), W(DS))

#define ornmx3rr(XD, XS, XT)                                                \
        notmx_rr(W(XD), W(XS))                                              \
        orrmx_rr(W(XD), W(XT))

#define ornmx3ld(XD, XS, MT, DT)                                            \
        notmx_rr(W(XD), W(XS))                                              \
        orrmx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xormx_rr(XG, XS)                                                    \
        xormx3rr(W(XG), W(XG), W(XS))

#define xormx_ld(XG, MS, DS)                                                \
        xormx3ld(W(XG), W(XG), W(MS), W(DS))

#define xormx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xEF)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xormx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xEF)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notmx_rx(XG)                                                        \
        notmx_rr(W(XG), W(XG))

#define notmx_rr(XD, XS)                                                    \
        annmx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

#if (RT_512X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define addmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define addmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsmx3rr(W(XG), W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define adsmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsmn3rr(W(XG), W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmn_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmn_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define adsmn_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        submx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define submx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        submx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define submx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsmx3rr(W(XG), W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsmn3rr(W(XG), W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmn_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmn_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define sbsmn_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulmx_rx(W(XD))                                                     \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define mulmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlax3ri(W(XD), W(XS), W(IT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shlax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shlax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrax3ri(W(XD), W(XS), W(IT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrax_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shrax3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shrax_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shran3ri(W(XD), W(XS), W(IT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shran_ri(W(XD), W(IT))                                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        shran3ld(W(XD), W(XS), W(MT), W(DT))                                \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        shran_ld(W(XD), W(MT), W(DT))                                       \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmx3rr(W(XG), W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svlmx_xx() /* not portable, do not use outside */                   \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmx3rr(W(XG), W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmx_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmx_xx() /* not portable, do not use outside */                   \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        stack_ld(Recx)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmn3rr(W(XG), W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmn_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmn_xx()                                                          \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define svrmn_xx() /* not portable, do not use outside */                   \
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
        movhx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x12))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x16))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x22))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x26))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x32))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x36))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x3E))                                    \
        stack_ld(Recx)

#else /* RT_512X1 >= 2 */

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xFD)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xFD)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsmx3rr(W(XG), W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDD)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDD)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #S) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsmn3rr(W(XG), W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xED)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xED)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xF9)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define submx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xF9)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsmx3rr(W(XG), W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xD9)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xD9)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #S) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsmn3rr(W(XG), W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xE9)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xE9)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xD5)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xD5)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmx_ri(XG, IS)                                                    \
        shlmx3ri(W(XG), W(XG), W(IS))

#define shlmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmx3ri(XD, XS, IT)                                                \
        EVX(REG(XD), K, 1, 1) EMITB(0x71)                                   \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xF1)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmx_ri(XG, IS)                                                    \
        shrmx3ri(W(XG), W(XG), W(IS))

#define shrmx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmx3ri(XD, XS, IT)                                                \
        EVX(REG(XD), K, 1, 1) EMITB(0x71)                                   \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xD1)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmn_ri(XG, IS)                                                    \
        shrmn3ri(W(XG), W(XG), W(IS))

#define shrmn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmn3ri(XD, XS, IT)                                                \
        EVX(REG(XD), K, 1, 1) EMITB(0x71)                                   \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrmn3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xE1)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmx3rr(W(XG), W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 2) EMITB(0x12)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 2) EMITB(0x12)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmx3rr(W(XG), W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 2) EMITB(0x10)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 2) EMITB(0x10)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmn3rr(W(XG), W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 2) EMITB(0x11)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 2) EMITB(0x11)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_512X1 >= 2 */

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
        movox_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        VEX(0x00, 0, 0, 1) EMITB(0x91)                                      \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_st(Redx)                                                      \
        stack_st(Recx)                                                      \
        stack_st(Rebx)                                                      \
        stack_st(Reax)                                                      \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        stack_ld(Reax)                                                      \
        andwz_ri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
        EMITB(0x74) EMITB(0x05)                                             \
        VEW(0x00, 0, 0, 1) EMITB(0x91)                                      \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_ld(Rebx)                                                      \
        stack_ld(Recx)                                                      \
        stack_ld(Redx)

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
        movox_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        VEX(0x00, 0, 0, 1) EMITB(0x90)                                      \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_st(Redx)                                                      \
        stack_st(Recx)                                                      \
        stack_st(Rebx)                                                      \
        stack_st(Reax)                                                      \
        movwx_ri(Reax, IB(7))                                               \
        movwx_ri(Recx, IB(0))                                               \
        cpuid_xx()                                                          \
        stack_ld(Reax)                                                      \
        andwz_ri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
        EMITB(0x74) EMITB(0x05)                                             \
        VEW(0x00, 0, 0, 1) EMITB(0x90)                                      \
        MRM(0x01,       0x00,    0x00)                                      \
        stack_ld(Rebx)                                                      \
        stack_ld(Recx)                                                      \
        stack_ld(Redx)

#endif /* RT_512X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_512X1V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
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

#if (RT_512X1 == 1) /* instructions below require AVX512BW (16/8-bit mx/mb) */

#define ck1qx_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVW(REG(XS), K, 1, 2) EMITB(0x29)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define ck1ox_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(REG(XS), K, 1, 1) EMITB(0x76)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define mz1qx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EZW(0x00,    K, 1, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mz1ox_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EZX(0x00,    K, 0, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define ck1mx_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(REG(XS), K, 1, 1) EMITB(0x75)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define ck1mb_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(REG(XS), K, 1, 1) EMITB(0x74)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define mz1mx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EZW(REG(XD), K, 1, 2) EMITB(0x66)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mz1mb_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EZX(REG(XD), K, 1, 2) EMITB(0x66)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#else  /* (RT_512X1 == 2) */

#define ck1qx_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVW(0x00,    K, 2, 2) EMITB(0x39)                                   \
        MRM(0x01,    MOD(XS), REG(XS))

#define ck1ox_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(0x00,    K, 2, 2) EMITB(0x39)                                   \
        MRM(0x01,    MOD(XS), REG(XS))

#define mz1qx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVW(0x00,    K, 2, 2) EMITB(0x38)                                   \
        MRM(REG(XD),    0x03,    0x01)

#define mz1ox_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVX(0x00,    K, 2, 2) EMITB(0x38)                                   \
        MRM(REG(XD),    0x03,    0x01)

#define ck1mx_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVW(0x00,    K, 2, 2) EMITB(0x29)                                   \
        MRM(0x01,    MOD(XS), REG(XS))

#define ck1mb_rm(XS, MT, DT) /* not portable, do not use outside */         \
        EVX(0x00,    K, 2, 2) EMITB(0x29)                                   \
        MRM(0x01,    MOD(XS), REG(XS))

#define mz1mx_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVW(0x00,    K, 2, 2) EMITB(0x28)                                   \
        MRM(REG(XD),    0x03,    0x01)

#define mz1mb_ld(XD, MS, DS) /* not portable, do not use outside */         \
        EVX(0x00,    K, 2, 2) EMITB(0x28)                                   \
        MRM(REG(XD),    0x03,    0x01)

#endif /* (RT_512X1 == 2) */

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

/***********   packed single/double-precision generic move/logic   ************/

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


#define movqx_rr(XD, XS)                                                    \
        EVW(0x00,    K, 1, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movqx_ld(XD, MS, DS)                                                \
        EVW(0x00,    K, 1, 1) EMITB(0x28)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movqx_st(XS, MD, DD)                                                \
        EVW(0x00,    K, 1, 1) EMITB(0x29)                                   \
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


#define mmvqx_rr(XG, XS)                                                    \
        ck1qx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(0x00,    K, 1, 1) EMITB(0x28)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvqx_ld(XG, MS, DS)                                                \
        ck1qx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(0x00,    K, 1, 1) EMITB(0x28)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvqx_st(XS, MG, DG)                                                \
        ck1qx_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKW(0x00,    K, 1, 1) EMITB(0x29)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#if (RT_512X1 < 2)

/* and (G = G & S), (D = S & T) if (#D != #T) */

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


#define andqx_rr(XG, XS)                                                    \
        andqx3rr(W(XG), W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        andqx3ld(W(XG), W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0xDB)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0xDB)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

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


#define annqx_rr(XG, XS)                                                    \
        annqx3rr(W(XG), W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        annqx3ld(W(XG), W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0xDF)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0xDF)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

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


#define orrqx_rr(XG, XS)                                                    \
        orrqx3rr(W(XG), W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrqx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0xEB)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0xEB)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

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


#define ornqx_rr(XG, XS)                                                    \
        notqx_rx(W(XG))                                                     \
        orrqx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        notqx_rx(W(XG))                                                     \
        orrqx_ld(W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        notqx_rr(W(XD), W(XS))                                              \
        orrqx_rr(W(XD), W(XT))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        notqx_rr(W(XD), W(XS))                                              \
        orrqx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

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


#define xorqx_rr(XG, XS)                                                    \
        xorqx3rr(W(XG), W(XG), W(XS))

#define xorqx_ld(XG, MS, DS)                                                \
        xorqx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0xEF)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0xEF)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#else /* RT_512X1 >= 2 */

/* and (G = G & S), (D = S & T) if (#D != #T) */

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


#define andqx_rr(XG, XS)                                                    \
        andqx3rr(W(XG), W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        andqx3ld(W(XG), W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x54)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x54)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

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


#define annqx_rr(XG, XS)                                                    \
        annqx3rr(W(XG), W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        annqx3ld(W(XG), W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x55)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x55)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

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


#define orrqx_rr(XG, XS)                                                    \
        orrqx3rr(W(XG), W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrqx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x56)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x56)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

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


#define ornqx_rr(XG, XS)                                                    \
        notqx_rx(W(XG))                                                     \
        orrqx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        notqx_rx(W(XG))                                                     \
        orrqx_ld(W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        notqx_rr(W(XD), W(XS))                                              \
        orrqx_rr(W(XD), W(XT))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        notqx_rr(W(XD), W(XS))                                              \
        orrqx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

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


#define xorqx_rr(XG, XS)                                                    \
        xorqx3rr(W(XG), W(XG), W(XS))

#define xorqx_ld(XG, MS, DS)                                                \
        xorqx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorqx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x57)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorqx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x57)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_512X1 >= 2 */

/* not (G = ~G), (D = ~S) */

#define notox_rx(XG)                                                        \
        notox_rr(W(XG), W(XG))

#define notox_rr(XD, XS)                                                    \
        annox3ld(W(XD), W(XS), Mebp, inf_GPC07)


#define notqx_rx(XG)                                                        \
        notqx_rr(W(XG), W(XG))

#define notqx_rr(XD, XS)                                                    \
        annqx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/********   packed single/double-precision floating-point arithmetic   ********/

/* neg (G = -G), (D = -S) */

#define negos_rx(XG)                                                        \
        negos_rr(W(XG), W(XG))

#define negos_rr(XD, XS)                                                    \
        xorox3ld(W(XD), W(XS), Mebp, inf_GPC06_32)


#define negqs_rx(XG)                                                        \
        negqs_rr(W(XG), W(XG))

#define negqs_rr(XD, XS)                                                    \
        xorqx3ld(W(XD), W(XS), Mebp, inf_GPC06_64)

/* add (G = G + S), (D = S + T) if (#D != #T) */

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


#define addqs_rr(XG, XS)                                                    \
        addqs3rr(W(XG), W(XG), W(XS))

#define addqs_ld(XG, MS, DS)                                                \
        addqs3ld(W(XG), W(XG), W(MS), W(DS))

#define addqs3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x58)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addqs3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x58)                                   \
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


#undef  adpqs_rx
#define adpqs_rx(XD) /* not portable, do not use outside */                 \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movjx_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movjx_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        adpjs_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        movjx_st(W(XD), Mebp, inf_SCR01(0x30))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

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


#define subqs_rr(XG, XS)                                                    \
        subqs3rr(W(XG), W(XG), W(XS))

#define subqs_ld(XG, MS, DS)                                                \
        subqs3ld(W(XG), W(XG), W(MS), W(DS))

#define subqs3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x5C)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subqs3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x5C)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

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


#define mulqs_rr(XG, XS)                                                    \
        mulqs3rr(W(XG), W(XG), W(XS))

#define mulqs_ld(XG, MS, DS)                                                \
        mulqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulqs3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x59)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulqs3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x59)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

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


#define divqs_rr(XG, XS)                                                    \
        divqs3rr(W(XG), W(XG), W(XS))

#define divqs_ld(XG, MS, DS)                                                \
        divqs3ld(W(XG), W(XG), W(MS), W(DS))

#define divqs3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 1) EMITB(0x5E)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divqs3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 1) EMITB(0x5E)                                   \
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


#define sqrqs_rr(XD, XS)                                                    \
        EVW(0x00,    K, 1, 1) EMITB(0x51)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrqs_ld(XD, MS, DS)                                                \
        EVW(0x00,    K, 1, 1) EMITB(0x51)                                   \
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

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

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

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

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

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

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

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

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

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

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

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

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

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

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

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

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

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

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

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnox_rr(XD, XS)     /* round towards near */                       \
        cvtox_rr(W(XD), W(XS))

#define cvnox_ld(XD, MS, DS) /* round towards near */                       \
        cvtox_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtox_rr(XD, XS)                                                    \
        EVX(0x00,    K, 3, 1) EMITB(0x7A)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtox_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 3, 1) EMITB(0x7A)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define ruzos_ld(XD, MS, DS) /* round towards zero */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cuzos_rr(XD, XS)     /* round towards zero */                       \
        EVX(0x00,    K, 0, 1) EMITB(0x78)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cuzos_ld(XD, MS, DS) /* round towards zero */                       \
        EVX(0x00,    K, 0, 1) EMITB(0x78)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupos_rr(XD, XS)     /* round towards +inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rupos_ld(XD, MS, DS) /* round towards +inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cupos_rr(XD, XS)     /* round towards +inf */                       \
        ERX(0x00,    2, 0, 1) EMITB(0x79)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cupos_ld(XD, MS, DS) /* round towards +inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cupos_rr(W(XD), W(XD))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumos_rr(XD, XS)     /* round towards -inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rumos_ld(XD, MS, DS) /* round towards -inf */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cumos_rr(XD, XS)     /* round towards -inf */                       \
        ERX(0x00,    1, 0, 1) EMITB(0x79)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cumos_ld(XD, MS, DS) /* round towards -inf */                       \
        movox_ld(W(XD), W(MS), W(DS))                                       \
        cumos_rr(W(XD), W(XD))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runos_rr(XD, XS)     /* round towards near */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define runos_ld(XD, MS, DS) /* round towards near */                       \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cunos_rr(XD, XS)     /* round towards near */                       \
        cutos_rr(W(XD), W(XS))

#define cunos_ld(XD, MS, DS) /* round towards near */                       \
        cutos_ld(W(XD), W(MS), W(DS))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudos_rr(XD, XS)                                                    \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rudos_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cutos_rr(XD, XS)                                                    \
        EVX(0x00,    K, 0, 1) EMITB(0x79)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cutos_ld(XD, MS, DS)                                                \
        EVX(0x00,    K, 0, 1) EMITB(0x79)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruros_rr(XD, XS, mode)                                              \
        EVX(0x00,    K, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define curos_rr(XD, XS, mode)                                              \
        ERX(0x00,    RT_SIMD_MODE_##mode&3, 0, 1) EMITB(0x79)               \
        MRM(REG(XD), MOD(XS), REG(XS))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

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

/* sub (G = G - S), (D = S - T) if (#D != #T) */

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

/* mul (G = G * S), (D = S * T) if (#D != #T) */

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

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
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

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
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

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
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

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
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

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
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

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
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

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minox_rr(XG, XS)                                                    \
        minox3rr(W(XG), W(XG), W(XS))

#define minox_ld(XG, MS, DS)                                                \
        minox3ld(W(XG), W(XG), W(MS), W(DS))

#define minox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x3B)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x3B)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minon_rr(XG, XS)                                                    \
        minon3rr(W(XG), W(XG), W(XS))

#define minon_ld(XG, MS, DS)                                                \
        minon3ld(W(XG), W(XG), W(MS), W(DS))

#define minon3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x39)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minon3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x39)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxox_rr(XG, XS)                                                    \
        maxox3rr(W(XG), W(XG), W(XS))

#define maxox_ld(XG, MS, DS)                                                \
        maxox3ld(W(XG), W(XG), W(MS), W(DS))

#define maxox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x3F)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x3F)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxon_rr(XG, XS)                                                    \
        maxon3rr(W(XG), W(XG), W(XS))

#define maxon_ld(XG, MS, DS)                                                \
        maxon3ld(W(XG), W(XG), W(MS), W(DS))

#define maxon3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x3D)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxon3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x3D)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqox_rr(XG, XS)                                                    \
        ceqox3rr(W(XG), W(XG), W(XS))

#define ceqox_ld(XG, MS, DS)                                                \
        ceqox3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define ceqox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneox_rr(XG, XS)                                                    \
        cneox3rr(W(XG), W(XG), W(XS))

#define cneox_ld(XG, MS, DS)                                                \
        cneox3ld(W(XG), W(XG), W(MS), W(DS))

#define cneox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cneox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltox_rr(XG, XS)                                                    \
        cltox3rr(W(XG), W(XG), W(XS))

#define cltox_ld(XG, MS, DS)                                                \
        cltox3ld(W(XG), W(XG), W(MS), W(DS))

#define cltox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cltox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define clton_rr(XG, XS)                                                    \
        clton3rr(W(XG), W(XG), W(XS))

#define clton_ld(XG, MS, DS)                                                \
        clton3ld(W(XG), W(XG), W(MS), W(DS))

#define clton3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define clton3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleox_rr(XG, XS)                                                    \
        cleox3rr(W(XG), W(XG), W(XS))

#define cleox_ld(XG, MS, DS)                                                \
        cleox3ld(W(XG), W(XG), W(MS), W(DS))

#define cleox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cleox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cleon_rr(XG, XS)                                                    \
        cleon3rr(W(XG), W(XG), W(XS))

#define cleon_ld(XG, MS, DS)                                                \
        cleon3ld(W(XG), W(XG), W(MS), W(DS))

#define cleon3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cleon3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtox_rr(XG, XS)                                                    \
        cgtox3rr(W(XG), W(XG), W(XS))

#define cgtox_ld(XG, MS, DS)                                                \
        cgtox3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cgtox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgton_rr(XG, XS)                                                    \
        cgton3rr(W(XG), W(XG), W(XS))

#define cgton_ld(XG, MS, DS)                                                \
        cgton3ld(W(XG), W(XG), W(MS), W(DS))

#define cgton3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cgton3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeox_rr(XG, XS)                                                    \
        cgeox3rr(W(XG), W(XG), W(XS))

#define cgeox_ld(XG, MS, DS)                                                \
        cgeox3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeox3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cgeox3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgeon_rr(XG, XS)                                                    \
        cgeon3rr(W(XG), W(XG), W(XS))

#define cgeon_ld(XG, MS, DS)                                                \
        cgeon3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeon3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

#define cgeon3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x1F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1ox_ld(W(XD), Mebp, inf_GPC07)

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

#endif /* RT_512X1 >= 2 */

/* and (G = G & S), (D = S & T) if (#D != #T) */

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

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

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

/* orr (G = G | S), (D = S | T) if (#D != #T) */

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

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

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

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmx_rr(XG, XS)                                                    \
        addmx3rr(W(XG), W(XG), W(XS))

#define addmx_ld(XG, MS, DS)                                                \
        addmx3ld(W(XG), W(XG), W(MS), W(DS))

#define addmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addmx_rx(W(XD))

#define addmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addmx_rx(W(XD))

#define addmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmx_rr(XG, XS)                                                    \
        adsmx3rr(W(XG), W(XG), W(XS))

#define adsmx_ld(XG, MS, DS)                                                \
        adsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmx_rx(W(XD))

#define adsmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmx_rx(W(XD))

#define adsmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmn_rr(XG, XS)                                                    \
        adsmn3rr(W(XG), W(XG), W(XS))

#define adsmn_ld(XG, MS, DS)                                                \
        adsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmn_rx(W(XD))

#define adsmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmn_rx(W(XD))

#define adsmn_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submx_rr(XG, XS)                                                    \
        submx3rr(W(XG), W(XG), W(XS))

#define submx_ld(XG, MS, DS)                                                \
        submx3ld(W(XG), W(XG), W(MS), W(DS))

#define submx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        submx_rx(W(XD))

#define submx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        submx_rx(W(XD))

#define submx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmx_rr(XG, XS)                                                    \
        sbsmx3rr(W(XG), W(XG), W(XS))

#define sbsmx_ld(XG, MS, DS)                                                \
        sbsmx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmx_rx(W(XD))

#define sbsmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmx_rx(W(XD))

#define sbsmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmn_rr(XG, XS)                                                    \
        sbsmn3rr(W(XG), W(XG), W(XS))

#define sbsmn_ld(XG, MS, DS)                                                \
        sbsmn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmn_rx(W(XD))

#define sbsmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmn_rx(W(XD))

#define sbsmn_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmx_rr(XG, XS)                                                    \
        mulmx3rr(W(XG), W(XG), W(XS))

#define mulmx_ld(XG, MS, DS)                                                \
        mulmx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulmx_rx(W(XD))

#define mulmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulmx_rx(W(XD))

#define mulmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mulax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
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

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
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

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
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

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmx3rr(W(XG), W(XG), W(XS))

#define svlmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlmx_rx(W(XD))

#define svlmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlmx_rx(W(XD))

#define svlmx_rx(XD) /* not portable, do not use outside */                 \
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
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmx3rr(W(XG), W(XG), W(XS))

#define svrmx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmx_rx(W(XD))

#define svrmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmx_rx(W(XD))

#define svrmx_rx(XD) /* not portable, do not use outside */                 \
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
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmn3rr(W(XG), W(XG), W(XS))

#define svrmn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmn_rx(W(XD))

#define svrmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmn_rx(W(XD))

#define svrmn_rx(XD) /* not portable, do not use outside */                 \
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
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_512X1 >= 2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

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

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

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

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

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

/* sub (G = G - S), (D = S - T) if (#D != #T) */

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

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

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

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

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

/* mul (G = G * S), (D = S * T) if (#D != #T) */

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

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
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

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
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

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
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

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
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

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
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

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
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

/*****************   packed half-precision integer compare   ******************/

#if (RT_512X1 < 2)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        minmx3rr(W(XG), W(XG), W(XS))

#define minmx_ld(XG, MS, DS)                                                \
        minmx3ld(W(XG), W(XG), W(MS), W(DS))

#define minmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmx_rx(W(XD))

#define minmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmx_rx(W(XD))

#define minmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        minmn3rr(W(XG), W(XG), W(XS))

#define minmn_ld(XG, MS, DS)                                                \
        minmn3ld(W(XG), W(XG), W(MS), W(DS))

#define minmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmn_rx(W(XD))

#define minmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmn_rx(W(XD))

#define minmn_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        maxmx3rr(W(XG), W(XG), W(XS))

#define maxmx_ld(XG, MS, DS)                                                \
        maxmx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmx_rx(W(XD))

#define maxmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmx_rx(W(XD))

#define maxmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        maxmn3rr(W(XG), W(XG), W(XS))

#define maxmn_ld(XG, MS, DS)                                                \
        maxmn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmn_rx(W(XD))

#define maxmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmn_rx(W(XD))

#define maxmn_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqmx3rr(W(XG), W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqmx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        ceqmx_rx(W(XD))

#define ceqmx3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        ceqmx_rx(W(XD))

#define ceqmx_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        ceqax_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtmn3rr(W(XG), W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        cgtmn_rx(W(XD))

#define cgtmn3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        cgtmn_rx(W(XD))

#define cgtmn_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        cgtan_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemx_rr(XG, XS)                                                    \
        cnemx3rr(W(XG), W(XG), W(XS))

#define cnemx_ld(XG, MS, DS)                                                \
        cnemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemx3rr(XD, XS, XT)                                                \
        ceqmx3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define cnemx3ld(XD, XS, MT, DT)                                            \
        ceqmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmx_rr(XG, XS)                                                    \
        cltmx3rr(W(XG), W(XG), W(XS))

#define cltmx_ld(XG, MS, DS)                                                \
        cltmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmx3rr(XD, XS, XT)                                                \
        minmx3rr(W(XD), W(XS), W(XT))                                       \
        cnemx_rr(W(XD), W(XT))

#define cltmx3ld(XD, XS, MT, DT)                                            \
        minmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmn_rr(XG, XS)                                                    \
        cltmn3rr(W(XG), W(XG), W(XS))

#define cltmn_ld(XG, MS, DS)                                                \
        cltmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmn3rr(XD, XS, XT)                                                \
        cgtmn3rr(W(XD), W(XT), W(XS))

#define cltmn3ld(XD, XS, MT, DT)                                            \
        minmn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemx_rr(XG, XS)                                                    \
        clemx3rr(W(XG), W(XG), W(XS))

#define clemx_ld(XG, MS, DS)                                                \
        clemx3ld(W(XG), W(XG), W(MS), W(DS))

#define clemx3rr(XD, XS, XT)                                                \
        maxmx3rr(W(XD), W(XS), W(XT))                                       \
        ceqmx_rr(W(XD), W(XT))

#define clemx3ld(XD, XS, MT, DT)                                            \
        maxmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemn_rr(XG, XS)                                                    \
        clemn3rr(W(XG), W(XG), W(XS))

#define clemn_ld(XG, MS, DS)                                                \
        clemn3ld(W(XG), W(XG), W(MS), W(DS))

#define clemn3rr(XD, XS, XT)                                                \
        cgtmn3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define clemn3ld(XD, XS, MT, DT)                                            \
        cgtmn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmx_rr(XG, XS)                                                    \
        cgtmx3rr(W(XG), W(XG), W(XS))

#define cgtmx_ld(XG, MS, DS)                                                \
        cgtmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmx3rr(XD, XS, XT)                                                \
        maxmx3rr(W(XD), W(XS), W(XT))                                       \
        cnemx_rr(W(XD), W(XT))

#define cgtmx3ld(XD, XS, MT, DT)                                            \
        maxmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemx_rr(XG, XS)                                                    \
        cgemx3rr(W(XG), W(XG), W(XS))

#define cgemx_ld(XG, MS, DS)                                                \
        cgemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemx3rr(XD, XS, XT)                                                \
        minmx3rr(W(XD), W(XS), W(XT))                                       \
        ceqmx_rr(W(XD), W(XT))

#define cgemx3ld(XD, XS, MT, DT)                                            \
        minmx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemn_rr(XG, XS)                                                    \
        cgemn3rr(W(XG), W(XG), W(XS))

#define cgemn_ld(XG, MS, DS)                                                \
        cgemn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemn3rr(XD, XS, XT)                                                \
        minmn3rr(W(XD), W(XS), W(XT))                                       \
        ceqmx_rr(W(XD), W(XT))

#define cgemn3ld(XD, XS, MT, DT)                                            \
        minmn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmx_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_512  0x00000000 /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_512  0xFFF0FFF0 /*  all satisfy the condition */

#define adpax3rr(XD, XS, XT)     /* not portable, do not use outside */     \
        VEX(REG(XS), 1, 1, 2) EMITB(0x01)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adpax3ld(XD, XS, MT, DT) /* not portable, do not use outside */     \
        VEX(REG(XS), 1, 1, 2) EMITB(0x01)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define mkjmx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        adpax3ld(W(XS), W(XS), Mebp, inf_SCR01(0x20))                       \
        adpax3rr(W(XS), W(XS), W(XS))                                       \
        adpax3rr(W(XS), W(XS), W(XS))                                       \
        adpax3rr(W(XS), W(XS), W(XS))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movmx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_mi(Mebp, inf_SCR02(0), IW(RT_SIMD_MASK_##mask##16_512))       \
        jeqxx_lb(lb)

#else /* RT_512X1 >= 2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmx_rr(XG, XS)                                                    \
        minmx3rr(W(XG), W(XG), W(XS))

#define minmx_ld(XG, MS, DS)                                                \
        minmx3ld(W(XG), W(XG), W(MS), W(DS))

#define minmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x3A)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x3A)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmn_rr(XG, XS)                                                    \
        minmn3rr(W(XG), W(XG), W(XS))

#define minmn_ld(XG, MS, DS)                                                \
        minmn3ld(W(XG), W(XG), W(MS), W(DS))

#define minmn3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xEA)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmn3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xEA)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmx_rr(XG, XS)                                                    \
        maxmx3rr(W(XG), W(XG), W(XS))

#define maxmx_ld(XG, MS, DS)                                                \
        maxmx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmx3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x3E)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmx3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x3E)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmn_rr(XG, XS)                                                    \
        maxmn3rr(W(XG), W(XG), W(XS))

#define maxmn_ld(XG, MS, DS)                                                \
        maxmn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmn3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xEE)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmn3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xEE)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmx_rr(XG, XS)                                                    \
        ceqmx3rr(W(XG), W(XG), W(XS))

#define ceqmx_ld(XG, MS, DS)                                                \
        ceqmx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define ceqmx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemx_rr(XG, XS)                                                    \
        cnemx3rr(W(XG), W(XG), W(XS))

#define cnemx_ld(XG, MS, DS)                                                \
        cnemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define cnemx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmx_rr(XG, XS)                                                    \
        cltmx3rr(W(XG), W(XG), W(XS))

#define cltmx_ld(XG, MS, DS)                                                \
        cltmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define cltmx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmn_rr(XG, XS)                                                    \
        cltmn3rr(W(XG), W(XG), W(XS))

#define cltmn_ld(XG, MS, DS)                                                \
        cltmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmn3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define cltmn3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemx_rr(XG, XS)                                                    \
        clemx3rr(W(XG), W(XG), W(XS))

#define clemx_ld(XG, MS, DS)                                                \
        clemx3ld(W(XG), W(XG), W(MS), W(DS))

#define clemx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define clemx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemn_rr(XG, XS)                                                    \
        clemn3rr(W(XG), W(XG), W(XS))

#define clemn_ld(XG, MS, DS)                                                \
        clemn3ld(W(XG), W(XG), W(MS), W(DS))

#define clemn3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define clemn3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmx_rr(XG, XS)                                                    \
        cgtmx3rr(W(XG), W(XG), W(XS))

#define cgtmx_ld(XG, MS, DS)                                                \
        cgtmx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define cgtmx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmn_rr(XG, XS)                                                    \
        cgtmn3rr(W(XG), W(XG), W(XS))

#define cgtmn_ld(XG, MS, DS)                                                \
        cgtmn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmn3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define cgtmn3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemx_rr(XG, XS)                                                    \
        cgemx3rr(W(XG), W(XG), W(XS))

#define cgemx_ld(XG, MS, DS)                                                \
        cgemx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemx3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define cgemx3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemn_rr(XG, XS)                                                    \
        cgemn3rr(W(XG), W(XG), W(XS))

#define cgemn_ld(XG, MS, DS)                                                \
        cgemn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemn3rr(XD, XS, XT)                                                \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

#define cgemn3ld(XD, XS, MT, DT)                                            \
        EVW(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1mx_ld(W(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_512  0x00000000 /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_512  0xFFFFFFFF /*  all satisfy the condition */

#define mk1hx_rx(RD)         /* not portable, do not use outside */         \
        VEX(0x00,    0, 3, 1) EMITB(0x93)                                   \
        MRM(REG(RD),    0x03,    0x01)

#define mkjmx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1mx_rm(W(XS), Mebp, inf_GPC07)                                    \
        mk1hx_rx(Reax)                                                      \
        cmpwx_ri(Reax, IW(RT_SIMD_MASK_##mask##16_512))                     \
        jeqxx_lb(lb)

#endif /* RT_512X1 >= 2 */

/****************   packed byte-precision generic move/logic   ****************/

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#if (RT_512X1 < 2)

#define mmvmb_rr(XG, XS)                                                    \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_rr(Xmm0, W(XG))                                               \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmx_rr(W(XG), Xmm0)

#define mmvmb_ld(XG, MS, DS)                                                \
        notmx_rx(Xmm0)                                                      \
        andmx_rr(W(XG), Xmm0)                                               \
        annmx_ld(Xmm0, W(MS), W(DS))                                        \
        orrmx_rr(W(XG), Xmm0)

#define mmvmb_st(XS, MG, DG)                                                \
        andmx_rr(W(XS), Xmm0)                                               \
        annmx_ld(Xmm0, W(MG), W(DG))                                        \
        orrmx_rr(Xmm0, W(XS))                                               \
        movmx_st(Xmm0, W(MG), W(DG))

#else /* RT_512X1 >= 2 */

#define mmvmb_rr(XG, XS)                                                    \
        ck1mb_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(0x00,    K, 3, 1) EMITB(0x6F)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mmvmb_ld(XG, MS, DS)                                                \
        ck1mb_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(0x00,    K, 3, 1) EMITB(0x6F)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvmb_st(XS, MG, DG)                                                \
        ck1mb_rm(Xmm0, Mebp, inf_GPC07)                                     \
        EKX(0x00,    K, 3, 1) EMITB(0x7F)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#endif /* RT_512X1 >= 2 */

/* move/logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

#if (RT_512X1 < 2)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmb_rr(XG, XS)                                                    \
        addmb3rr(W(XG), W(XG), W(XS))

#define addmb_ld(XG, MS, DS)                                                \
        addmb3ld(W(XG), W(XG), W(MS), W(DS))

#define addmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        addmb_rx(W(XD))

#define addmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        addmb_rx(W(XD))

#define addmb_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        addab_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        addab_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmb_rr(XG, XS)                                                    \
        adsmb3rr(W(XG), W(XG), W(XS))

#define adsmb_ld(XG, MS, DS)                                                \
        adsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmb_rx(W(XD))

#define adsmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmb_rx(W(XD))

#define adsmb_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsab_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsab_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmc_rr(XG, XS)                                                    \
        adsmc3rr(W(XG), W(XG), W(XS))

#define adsmc_ld(XG, MS, DS)                                                \
        adsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmc3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adsmc_rx(W(XD))

#define adsmc3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adsmc_rx(W(XD))

#define adsmc_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        adsac_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        adsac_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submb_rr(XG, XS)                                                    \
        submb3rr(W(XG), W(XG), W(XS))

#define submb_ld(XG, MS, DS)                                                \
        submb3ld(W(XG), W(XG), W(MS), W(DS))

#define submb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        submb_rx(W(XD))

#define submb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        submb_rx(W(XD))

#define submb_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        subab_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        subab_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmb_rr(XG, XS)                                                    \
        sbsmb3rr(W(XG), W(XG), W(XS))

#define sbsmb_ld(XG, MS, DS)                                                \
        sbsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmb_rx(W(XD))

#define sbsmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmb_rx(W(XD))

#define sbsmb_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsab_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsab_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmc_rr(XG, XS)                                                    \
        sbsmc3rr(W(XG), W(XG), W(XS))

#define sbsmc_ld(XG, MS, DS)                                                \
        sbsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmc3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        sbsmc_rx(W(XD))

#define sbsmc3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        sbsmc_rx(W(XD))

#define sbsmc_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        sbsac_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        sbsac_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_512X1 >= 2 */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addmb_rr(XG, XS)                                                    \
        addmb3rr(W(XG), W(XG), W(XS))

#define addmb_ld(XG, MS, DS)                                                \
        addmb3ld(W(XG), W(XG), W(MS), W(DS))

#define addmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xFC)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xFC)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsmb_rr(XG, XS)                                                    \
        adsmb3rr(W(XG), W(XG), W(XS))

#define adsmb_ld(XG, MS, DS)                                                \
        adsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDC)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDC)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsmc_rr(XG, XS)                                                    \
        adsmc3rr(W(XG), W(XG), W(XS))

#define adsmc_ld(XG, MS, DS)                                                \
        adsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsmc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xEC)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsmc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xEC)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define submb_rr(XG, XS)                                                    \
        submb3rr(W(XG), W(XG), W(XS))

#define submb_ld(XG, MS, DS)                                                \
        submb3ld(W(XG), W(XG), W(MS), W(DS))

#define submb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xF8)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define submb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xF8)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsmb_rr(XG, XS)                                                    \
        sbsmb3rr(W(XG), W(XG), W(XS))

#define sbsmb_ld(XG, MS, DS)                                                \
        sbsmb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xD8)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xD8)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsmc_rr(XG, XS)                                                    \
        sbsmc3rr(W(XG), W(XG), W(XS))

#define sbsmc_ld(XG, MS, DS)                                                \
        sbsmc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsmc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xE8)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsmc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xE8)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_512X1 >= 2 */

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulmb_rr(XG, XS)                                                    \
        mulmb3rr(W(XG), W(XG), W(XS))

#define mulmb_ld(XG, MS, DS)                                                \
        mulmb3ld(W(XG), W(XG), W(MS), W(DS))

#define mulmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulmb_rx(W(XD))

#define mulmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulmb_rx(W(XD))

#define mulmb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x01))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x01))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x02))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x02))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x03))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x03))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x05))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x05))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x06))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x06))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x07))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x07))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x09))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x09))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0F))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x11))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x11))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x12))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x12))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x13))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x13))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x14))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x14))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x15))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x15))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x16))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x16))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x17))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x17))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x19))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x19))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x1F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x1F))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x20))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x20))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x21))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x21))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x22))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x22))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x23))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x23))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x24))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x24))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x25))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x25))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x26))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x26))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x27))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x27))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x28))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x28))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x29))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x29))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x2F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x2F))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x30))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x30))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x31))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x31))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x32))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x32))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x33))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x33))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x34))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x34))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x35))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x35))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x36))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x36))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x37))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x37))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x38))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x38))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x39))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x39))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x3F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x3F))                              \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlmb_ri(XG, IS)                                                    \
        shlmb3ri(W(XG), W(XG), W(IS))

#define shlmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlmb3ld(W(XG), W(XG), W(MS), W(DS))

#define shlmb3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlmb_xx() /* not portable, do not use outside */                   \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x3F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmb_ri(XG, IS)                                                    \
        shrmb3ri(W(XG), W(XG), W(IS))

#define shrmb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmb3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmb3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrmb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmb_xx() /* not portable, do not use outside */                   \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x3F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrmc_ri(XG, IS)                                                    \
        shrmc3ri(W(XG), W(XG), W(IS))

#define shrmc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrmc3ld(W(XG), W(XG), W(MS), W(DS))

#define shrmc3ri(XD, XS, IT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrmc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmc3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrmc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrmc_xx() /* not portable, do not use outside */                   \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x11))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x12))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x13))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x14))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x15))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x16))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x17))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x19))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x1F))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x20))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x21))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x22))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x23))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x24))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x25))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x26))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x27))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x28))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x29))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x2F))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x30))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x31))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x32))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x33))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x34))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x35))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x36))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x37))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x38))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x39))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x3F))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlmb3rr(W(XG), W(XG), W(XS))

#define svlmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlmb3ld(W(XG), W(XG), W(MS), W(DS))

#define svlmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlmb_rx(W(XD))

#define svlmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlmb_rx(W(XD))

#define svlmb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x3F))                                    \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmb3rr(W(XG), W(XG), W(XS))

#define svrmb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmb3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmb_rx(W(XD))

#define svrmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmb_rx(W(XD))

#define svrmb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x1F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x21))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x22))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x23))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x25))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x26))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x27))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x29))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x2F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x31))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x32))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x33))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x35))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x36))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x37))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x39))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x3F))                                    \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrmc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrmc3rr(W(XG), W(XG), W(XS))

#define svrmc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrmc3ld(W(XG), W(XG), W(MS), W(DS))

#define svrmc3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrmc_rx(W(XD))

#define svrmc3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrmc_rx(W(XD))

#define svrmc_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x11))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x11))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x12))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x12))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x13))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x13))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x15))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x15))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x16))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x16))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x17))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x17))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x19))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x19))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x1F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x1F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x21))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x21))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x22))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x22))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x23))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x23))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x25))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x25))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x26))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x26))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x27))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x27))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x29))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x29))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x2F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x2F))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x31))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x31))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x32))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x32))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x33))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x33))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x35))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x35))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x36))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x36))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x37))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x37))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x39))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x39))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x3F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x3F))                                    \
        stack_ld(Recx)                                                      \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

#if (RT_512X1 < 2)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmb_rr(XG, XS)                                                    \
        minmb3rr(W(XG), W(XG), W(XS))

#define minmb_ld(XG, MS, DS)                                                \
        minmb3ld(W(XG), W(XG), W(MS), W(DS))

#define minmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmb_rx(W(XD))

#define minmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmb_rx(W(XD))

#define minmb_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minab_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minab_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmc_rr(XG, XS)                                                    \
        minmc3rr(W(XG), W(XG), W(XS))

#define minmc_ld(XG, MS, DS)                                                \
        minmc3ld(W(XG), W(XG), W(MS), W(DS))

#define minmc3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        minmc_rx(W(XD))

#define minmc3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        minmc_rx(W(XD))

#define minmc_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minac_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        minac_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmb_rr(XG, XS)                                                    \
        maxmb3rr(W(XG), W(XG), W(XS))

#define maxmb_ld(XG, MS, DS)                                                \
        maxmb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmb_rx(W(XD))

#define maxmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmb_rx(W(XD))

#define maxmb_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxab_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxab_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmc_rr(XG, XS)                                                    \
        maxmc3rr(W(XG), W(XG), W(XS))

#define maxmc_ld(XG, MS, DS)                                                \
        maxmc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmc3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        maxmc_rx(W(XD))

#define maxmc3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        maxmc_rx(W(XD))

#define maxmc_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxac_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxac_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmb_rr(XG, XS)                                                    \
        ceqmb3rr(W(XG), W(XG), W(XS))

#define ceqmb_ld(XG, MS, DS)                                                \
        ceqmb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmb3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        ceqmb_rx(W(XD))

#define ceqmb3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        ceqmb_rx(W(XD))

#define ceqmb_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        ceqab_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        ceqab_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmc_rr(XG, XS)                                                    \
        cgtmc3rr(W(XG), W(XG), W(XS))

#define cgtmc_ld(XG, MS, DS)                                                \
        cgtmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmc3rr(XD, XS, XT)                                                \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        cgtmc_rx(W(XD))

#define cgtmc3ld(XD, XS, MT, DT)                                            \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movmx_ld(W(XD), W(MT), W(DT))                                       \
        movmx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        cgtmc_rx(W(XD))

#define cgtmc_rx(XD) /* not portable, do not use outside */                 \
        movax_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        cgtac_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movax_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        cgtac_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        movax_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movmx_ld(W(XD), Mebp, inf_SCR01(0))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemb_rr(XG, XS)                                                    \
        cnemb3rr(W(XG), W(XG), W(XS))

#define cnemb_ld(XG, MS, DS)                                                \
        cnemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemb3rr(XD, XS, XT)                                                \
        ceqmb3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define cnemb3ld(XD, XS, MT, DT)                                            \
        ceqmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmb_rr(XG, XS)                                                    \
        cltmb3rr(W(XG), W(XG), W(XS))

#define cltmb_ld(XG, MS, DS)                                                \
        cltmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmb3rr(XD, XS, XT)                                                \
        minmb3rr(W(XD), W(XS), W(XT))                                       \
        cnemb_rr(W(XD), W(XT))

#define cltmb3ld(XD, XS, MT, DT)                                            \
        minmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemb_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmc_rr(XG, XS)                                                    \
        cltmc3rr(W(XG), W(XG), W(XS))

#define cltmc_ld(XG, MS, DS)                                                \
        cltmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmc3rr(XD, XS, XT)                                                \
        cgtmc3rr(W(XD), W(XT), W(XS))

#define cltmc3ld(XD, XS, MT, DT)                                            \
        minmc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemb_rr(XG, XS)                                                    \
        clemb3rr(W(XG), W(XG), W(XS))

#define clemb_ld(XG, MS, DS)                                                \
        clemb3ld(W(XG), W(XG), W(MS), W(DS))

#define clemb3rr(XD, XS, XT)                                                \
        maxmb3rr(W(XD), W(XS), W(XT))                                       \
        ceqmb_rr(W(XD), W(XT))

#define clemb3ld(XD, XS, MT, DT)                                            \
        maxmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemc_rr(XG, XS)                                                    \
        clemc3rr(W(XG), W(XG), W(XS))

#define clemc_ld(XG, MS, DS)                                                \
        clemc3ld(W(XG), W(XG), W(MS), W(DS))

#define clemc3rr(XD, XS, XT)                                                \
        cgtmc3rr(W(XD), W(XS), W(XT))                                       \
        notmx_rx(W(XD))

#define clemc3ld(XD, XS, MT, DT)                                            \
        cgtmc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notmx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmb_rr(XG, XS)                                                    \
        cgtmb3rr(W(XG), W(XG), W(XS))

#define cgtmb_ld(XG, MS, DS)                                                \
        cgtmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmb3rr(XD, XS, XT)                                                \
        maxmb3rr(W(XD), W(XS), W(XT))                                       \
        cnemb_rr(W(XD), W(XT))

#define cgtmb3ld(XD, XS, MT, DT)                                            \
        maxmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnemb_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemb_rr(XG, XS)                                                    \
        cgemb3rr(W(XG), W(XG), W(XS))

#define cgemb_ld(XG, MS, DS)                                                \
        cgemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemb3rr(XD, XS, XT)                                                \
        minmb3rr(W(XD), W(XS), W(XT))                                       \
        ceqmb_rr(W(XD), W(XT))

#define cgemb3ld(XD, XS, MT, DT)                                            \
        minmb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmb_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemc_rr(XG, XS)                                                    \
        cgemc3rr(W(XG), W(XG), W(XS))

#define cgemc_ld(XG, MS, DS)                                                \
        cgemc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemc3rr(XD, XS, XT)                                                \
        minmc3rr(W(XD), W(XS), W(XT))                                       \
        ceqmb_rr(W(XD), W(XT))

#define cgemc3ld(XD, XS, MT, DT)                                            \
        minmc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqmb_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_512    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_512    0xFF     /*  all satisfy the condition */

#define prmox_rx(XG) /* not portable, do not use outside */                 \
        EVX(REG(XG), K, 1, 3) EMITB(0x43)                                   \
        MRM(REG(XG), MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x4E))  /* permute two 256-bit halves */

/* #define bsncx_rx(XS, mask)              (defined in 86_256-bit header) */

#define mkjmb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movmx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Rebp)                                                      \
        V2X(0x00,    1, 0) EMITB(0x50)                                      \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        prmox_rx(W(XS))                                                     \
        bsncx_rx(W(XS), mask)                                               \
        shlox_ri(W(XS), IB(8))                                              \
        bsncx_rx(W(XS), mask)                                               \
        prmox_rx(W(XS))                                                     \
        bsncx_rx(W(XS), mask)                                               \
        shlox_ri(W(XS), IB(8))                                              \
        bsncx_rx(W(XS), mask)                                               \
        prmox_rx(W(XS))                                                     \
        bsncx_rx(W(XS), mask)                                               \
        shlox_ri(W(XS), IB(8))                                              \
        bsncx_rx(W(XS), mask)                                               \
        prmox_rx(W(XS))                                                     \
        bsncx_rx(W(XS), mask)                                               \
        stack_ld(Rebp)                                                      \
        movmx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##08_512))                     \
        jeqxx_lb(lb)

#else /* RT_512X1 >= 2 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minmb_rr(XG, XS)                                                    \
        minmb3rr(W(XG), W(XG), W(XS))

#define minmb_ld(XG, MS, DS)                                                \
        minmb3ld(W(XG), W(XG), W(MS), W(DS))

#define minmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDA)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDA)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minmc_rr(XG, XS)                                                    \
        minmc3rr(W(XG), W(XG), W(XS))

#define minmc_ld(XG, MS, DS)                                                \
        minmc3ld(W(XG), W(XG), W(MS), W(DS))

#define minmc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x38)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minmc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x38)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxmb_rr(XG, XS)                                                    \
        maxmb3rr(W(XG), W(XG), W(XS))

#define maxmb_ld(XG, MS, DS)                                                \
        maxmb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 1) EMITB(0xDE)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 1) EMITB(0xDE)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxmc_rr(XG, XS)                                                    \
        maxmc3rr(W(XG), W(XG), W(XS))

#define maxmc_ld(XG, MS, DS)                                                \
        maxmc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxmc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 2) EMITB(0x3C)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxmc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 2) EMITB(0x3C)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqmb_rr(XG, XS)                                                    \
        ceqmb3rr(W(XG), W(XG), W(XS))

#define ceqmb_ld(XG, MS, DS)                                                \
        ceqmb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define ceqmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnemb_rr(XG, XS)                                                    \
        cnemb3rr(W(XG), W(XG), W(XS))

#define cnemb_ld(XG, MS, DS)                                                \
        cnemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnemb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define cnemb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltmb_rr(XG, XS)                                                    \
        cltmb3rr(W(XG), W(XG), W(XS))

#define cltmb_ld(XG, MS, DS)                                                \
        cltmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define cltmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltmc_rr(XG, XS)                                                    \
        cltmc3rr(W(XG), W(XG), W(XS))

#define cltmc_ld(XG, MS, DS)                                                \
        cltmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltmc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define cltmc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clemb_rr(XG, XS)                                                    \
        clemb3rr(W(XG), W(XG), W(XS))

#define clemb_ld(XG, MS, DS)                                                \
        clemb3ld(W(XG), W(XG), W(MS), W(DS))

#define clemb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define clemb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clemc_rr(XG, XS)                                                    \
        clemc3rr(W(XG), W(XG), W(XS))

#define clemc_ld(XG, MS, DS)                                                \
        clemc3ld(W(XG), W(XG), W(MS), W(DS))

#define clemc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define clemc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtmb_rr(XG, XS)                                                    \
        cgtmb3rr(W(XG), W(XG), W(XS))

#define cgtmb_ld(XG, MS, DS)                                                \
        cgtmb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define cgtmb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtmc_rr(XG, XS)                                                    \
        cgtmc3rr(W(XG), W(XG), W(XS))

#define cgtmc_ld(XG, MS, DS)                                                \
        cgtmc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtmc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define cgtmc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgemb_rr(XG, XS)                                                    \
        cgemb3rr(W(XG), W(XG), W(XS))

#define cgemb_ld(XG, MS, DS)                                                \
        cgemb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemb3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define cgemb3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3E)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgemc_rr(XG, XS)                                                    \
        cgemc3rr(W(XG), W(XG), W(XS))

#define cgemc_ld(XG, MS, DS)                                                \
        cgemc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgemc3rr(XD, XS, XT)                                                \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

#define cgemc3ld(XD, XS, MT, DT)                                            \
        EVX(REG(XS), K, 1, 3) EMITB(0x3F)                                   \
        MRM(0x01,    MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))                                  \
        mz1mb_ld(W(XD), Mebp, inf_GPC07)

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_512  0x00000000 /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_512  0xFFFFFFFF /*  all satisfy the condition */

#define sh1hx_xx()           /* not portable, do not use outside */         \
        VEW(0x00,    0, 1, 3) EMITB(0x31)                                   \
        MRM(0x01,       0x03,    0x01)                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x20))

#define mkjmb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        ck1mb_rm(W(XS), Mebp, inf_GPC07)                                    \
        stack_st(Rebp)                                                      \
        mk1hx_rx(Reax)                                                      \
        sh1hx_xx()                                                          \
        mk1hx_rx(Rebp)                                                      \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##08_512 & 0x1) << 1)))  \
        MRM(0x00,       0x03, 0x05)                                         \
        stack_ld(Rebp)                                                      \
        cmpwx_ri(Reax, IW(RT_SIMD_MASK_##mask##08_512))                     \
        jeqxx_lb(lb)

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
        andwxZri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
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
        andwxZri(Rebx, IV(0x40000000))  /* check AVX512BW extension-bit */  \
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

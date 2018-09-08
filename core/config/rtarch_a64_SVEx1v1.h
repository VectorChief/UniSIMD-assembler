/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_SVEX1V1_H
#define RT_RTARCH_A64_SVEX1V1_H

#include "rtarch_a32_SVEx1v1.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64_SVEx1v1.h: Implementation of ARMv8:AArch64 fp64 SVE instructions.
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

#if (RT_SVEX1 != 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SVE   ***********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmqx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movts_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movqx_rr(XD, XS)                                                    \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), REG(XS)))

#define movqx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(REG(XD), MOD(MS), VAL(DS), B1(DS), F1(DS)))

#define movqx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xE5804000 | MPM(REG(XS), MOD(MD), VAL(DD), B1(DD), F1(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked elems */

#define mmvqx_rr(XG, XS)                                                    \
        EMITW(0x24C0A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x05E0C400 | MXM(REG(XG), REG(XS), REG(XG)))

#define mmvqx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x24C0A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x05E0C400 | MXM(REG(XG), TmmM,    REG(XG)))

#define mmvqx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x24C0A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x05E0C400 | MXM(TmmM,    REG(XS), TmmM))                     \
        EMITW(0xE5804000 | MPM(TmmM,    MOD(MG), VAL(DG), B1(DG), F1(DG)))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andqx_rr(XG, XS)                                                    \
        andqx3rr(W(XG), W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        andqx3ld(W(XG), W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        EMITW(0x04203000 | MXM(REG(XD), REG(XS), REG(XT)))

#define andqx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04203000 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annqx_rr(XG, XS)                                                    \
        annqx3rr(W(XG), W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        annqx3ld(W(XG), W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        EMITW(0x04E03000 | MXM(REG(XD), REG(XT), REG(XS)))

#define annqx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04E03000 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrqx_rr(XG, XS)                                                    \
        orrqx3rr(W(XG), W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrqx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorqx_rr(XG, XS)                                                    \
        xorqx3rr(W(XG), W(XG), W(XS))

#define xorqx_ld(XG, MS, DS)                                                \
        xorqx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorqx3rr(XD, XS, XT)                                                \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorqx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notqx_rx(XG)                                                        \
        notqx_rr(W(XG), W(XG))

#define notqx_rr(XD, XS)                                                    \
        EMITW(0x04DEA000 | MXM(REG(XD), REG(XS), 0x00))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negqs_rx(XG)                                                        \
        negqs_rr(W(XG), W(XG))

#define negqs_rr(XD, XS)                                                    \
        EMITW(0x04DDA000 | MXM(REG(XD), REG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addqs_rr(XG, XS)                                                    \
        addqs3rr(W(XG), W(XG), W(XS))

#define addqs_ld(XG, MS, DS)                                                \
        addqs3ld(W(XG), W(XG), W(MS), W(DS))

#define addqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C00000 | MXM(REG(XD), REG(XS), REG(XT)))

#define addqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C00000 | MXM(REG(XD), REG(XS), TmmM))

#define adpqs_rr(XG, XS) /* horizontal pairwise add, first 15-regs only */  \
        adpqs3rr(W(XG), W(XG), W(XS))

#define adpqs_ld(XG, MS, DS)                                                \
        adpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define adpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))

#define adpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        adpqs_rx(W(XD))

#define adhqs_rr(XD, XS) /* horizontal reductive add, first 15-regs only */ \
        adpqs3rr(W(XD), W(XS), W(XS))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))                                       \
        adpqs3rr(W(XD), W(XD), W(XD))

#define adhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        adhqs_rr(W(XD), W(XD))

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
        movjx_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subqs_rr(XG, XS)                                                    \
        subqs3rr(W(XG), W(XG), W(XS))

#define subqs_ld(XG, MS, DS)                                                \
        subqs3ld(W(XG), W(XG), W(MS), W(DS))

#define subqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C00400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C00400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulqs_rr(XG, XS)                                                    \
        mulqs3rr(W(XG), W(XG), W(XS))

#define mulqs_ld(XG, MS, DS)                                                \
        mulqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C00800 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C00800 | MXM(REG(XD), REG(XS), TmmM))

#define mlpqs_rr(XG, XS) /* horizontal pairwise mul */                      \
        mlpqs3rr(W(XG), W(XG), W(XS))

#define mlpqs_ld(XG, MS, DS)                                                \
        mlpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mlpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))

#define mlpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mlpqs_rx(W(XD))

#define mlhqs_rr(XD, XS) /* horizontal reductive mul */                     \
        mlpqs3rr(W(XD), W(XS), W(XS))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))                                       \
        mlpqs3rr(W(XD), W(XD), W(XD))

#define mlhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mlhqs_rr(W(XD), W(XD))

#define mlpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mults_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divqs_rr(XG, XS)                                                    \
        EMITW(0x65CD8000 | MXM(REG(XG), REG(XS), 0x00))

#define divqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65CD8000 | MXM(REG(XG), TmmM,    0x00))

#define divqs3rr(XD, XS, XT)                                                \
        movqx_rr(W(XD), W(XS))                                              \
        divqs_rr(W(XD), W(XT))

#define divqs3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        divqs_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqrqs_rr(XD, XS)                                                    \
        EMITW(0x65CDA000 | MXM(REG(XD), REG(XS), 0x00))

#define sqrqs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65CDA000 | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceqs_rr(XD, XS)                                                    \
        EMITW(0x65CE3000 | MXM(REG(XD), REG(XS), 0x00))

#define rcsqs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x65C01800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65C00800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseqs_rr(XD, XS)                                                    \
        EMITW(0x65CF3000 | MXM(REG(XD), REG(XS), 0x00))

#define rssqs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x65C00800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65C01C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65C00800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaqs_rr(XG, XS, XT)                                                \
        EMITW(0x65E00000 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65E00000 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsqs_rr(XG, XS, XT)                                                \
        EMITW(0x65E02000 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65E02000 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minqs_rr(XG, XS)                                                    \
        EMITW(0x65C78000 | MXM(REG(XG), REG(XS), 0x00))

#define minqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65C78000 | MXM(REG(XG), TmmM,    0x00))

#define minqs3rr(XD, XS, XT)                                                \
        movqx_rr(W(XD), W(XS))                                              \
        minqs_rr(W(XD), W(XT))

#define minqs3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        minqs_ld(W(XD), W(MT), W(DT))

#define mnpqs_rr(XG, XS) /* horizontal pairwise min */                      \
        mnpqs3rr(W(XG), W(XG), W(XS))

#define mnpqs_ld(XG, MS, DS)                                                \
        mnpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mnpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))

#define mnpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mnpqs_rx(W(XD))

#define mnhqs_rr(XD, XS) /* horizontal reductive min */                     \
        mnpqs3rr(W(XD), W(XS), W(XS))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))                                       \
        mnpqs3rr(W(XD), W(XD), W(XD))

#define mnhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mnhqs_rr(W(XD), W(XD))

#define mnpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        mints_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxqs_rr(XG, XS)                                                    \
        EMITW(0x65C68000 | MXM(REG(XG), REG(XS), 0x00))

#define maxqs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65C68000 | MXM(REG(XG), TmmM,    0x00))

#define maxqs3rr(XD, XS, XT)                                                \
        movqx_rr(W(XD), W(XS))                                              \
        maxqs_rr(W(XD), W(XT))

#define maxqs3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        maxqs_ld(W(XD), W(MT), W(DT))

#define mxpqs_rr(XG, XS) /* horizontal pairwise max */                      \
        mxpqs3rr(W(XG), W(XG), W(XS))

#define mxpqs_ld(XG, MS, DS)                                                \
        mxpqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mxpqs3rr(XD, XS, XT)                                                \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))

#define mxpqs3ld(XD, XS, MT, DT)                                            \
        movqx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XD), W(MT), W(DT))                                       \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mxpqs_rx(W(XD))

#define mxhqs_rr(XD, XS) /* horizontal reductive max */                     \
        mxpqs3rr(W(XD), W(XS), W(XS))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))                                       \
        mxpqs3rr(W(XD), W(XD), W(XD))

#define mxhqs_ld(XD, MS, DS)                                                \
        movqx_ld(W(XD), W(MS), W(DS))                                       \
        mxhqs_rr(W(XD), W(XD))

#define mxpqs_rx(XD) /* not portable, do not use outside */                 \
        movts_ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x10))                              \
        movts_ld(W(XD), Mebp, inf_SCR01(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR01(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x18))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x20))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x10))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x18))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x28))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x20))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x28))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x30))                              \
        movts_ld(W(XD), Mebp, inf_SCR02(0x30))                              \
        maxts_ld(W(XD), Mebp, inf_SCR02(0x38))                              \
        movts_st(W(XD), Mebp, inf_SCR01(0x38))                              \
        movqx_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqqs_rr(XG, XS)                                                    \
        ceqqs3rr(W(XG), W(XG), W(XS))

#define ceqqs_ld(XG, MS, DS)                                                \
        ceqqs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C06000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define ceqqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C06000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneqs_rr(XG, XS)                                                    \
        cneqs3rr(W(XG), W(XG), W(XS))

#define cneqs_ld(XG, MS, DS)                                                \
        cneqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cneqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C06010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cneqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C06010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltqs_rr(XG, XS)                                                    \
        cltqs3rr(W(XG), W(XG), W(XS))

#define cltqs_ld(XG, MS, DS)                                                \
        cltqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C04010 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cltqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C04010 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleqs_rr(XG, XS)                                                    \
        cleqs3rr(W(XG), W(XG), W(XS))

#define cleqs_ld(XG, MS, DS)                                                \
        cleqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cleqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C04000 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cleqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C04000 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtqs_rr(XG, XS)                                                    \
        cgtqs3rr(W(XG), W(XG), W(XS))

#define cgtqs_ld(XG, MS, DS)                                                \
        cgtqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C04010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cgtqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C04010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeqs_rr(XG, XS)                                                    \
        cgeqs3rr(W(XG), W(XG), W(XS))

#define cgeqs_ld(XG, MS, DS)                                                \
        cgeqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeqs3rr(XD, XS, XT)                                                \
        EMITW(0x65C04000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cgeqs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65C04000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05E0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_SVE     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_SVE     0x01    /*  all satisfy the condition */

#define mkjqx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x04982000 | MXM(TmmM,    REG(XS), 0x00)|                     \
                       RT_SIMD_MASK_##mask##64_SVE<<17)                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask##64_SVE))                     \
        jezxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x65C3A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnzqs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65C3A000 | MXM(REG(XD), TmmM,    0x00))

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x65DEA000 | MXM(REG(XD), REG(XS), 0x00))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65DEA000 | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x65C1A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65C1A000 | MXM(REG(XD), TmmM,    0x00))

#define cvpqs_rr(XD, XS)     /* round towards +inf */                       \
        rnpqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmqs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x65C2A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65C2A000 | MXM(REG(XD), TmmM,    0x00))

#define cvmqs_rr(XD, XS)     /* round towards -inf */                       \
        rnmqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnqs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x65C0A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnnqs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65C0A000 | MXM(REG(XD), TmmM,    0x00))

#define cvnqs_rr(XD, XS)     /* round towards near */                       \
        rnnqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvnqs_ld(XD, MS, DS) /* round towards near */                       \
        rnnqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnqn_rr(XD, XS)     /* round towards near */                       \
        cvtqn_rr(W(XD), W(XS))

#define cvnqn_ld(XD, MS, DS) /* round towards near */                       \
        cvtqn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        EMITW(0x65C7A000 | MXM(REG(XD), REG(XS), 0x00))

#define rndqs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65C7A000 | MXM(REG(XD), TmmM,    0x00))

#define cvtqs_rr(XD, XS)                                                    \
        rndqs_rr(W(XD), W(XS))                                              \
        cvzqs_rr(W(XD), W(XD))

#define cvtqs_ld(XD, MS, DS)                                                \
        rndqs_ld(W(XD), W(MS), W(DS))                                       \
        cvzqs_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtqn_rr(XD, XS)                                                    \
        EMITW(0x65D6A000 | MXM(REG(XD), REG(XS), 0x00))

#define cvtqn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65D6A000 | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        EMITW(0x65C0A000 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                        RT_SIMD_MODE_##mode << 16)

#define cvrqs_rr(XD, XS, mode)                                              \
        rnrqs_rr(W(XD), W(XS), mode)                                        \
        cvzqs_rr(W(XD), W(XD))

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addqx_rr(XG, XS)                                                    \
        addqx3rr(W(XG), W(XG), W(XS))

#define addqx_ld(XG, MS, DS)                                                \
        addqx3ld(W(XG), W(XG), W(MS), W(DS))

#define addqx3rr(XD, XS, XT)                                                \
        EMITW(0x04E00000 | MXM(REG(XD), REG(XS), REG(XT)))

#define addqx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04E00000 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subqx_rr(XG, XS)                                                    \
        subqx3rr(W(XG), W(XG), W(XS))

#define subqx_ld(XG, MS, DS)                                                \
        subqx3ld(W(XG), W(XG), W(MS), W(DS))

#define subqx3rr(XD, XS, XT)                                                \
        EMITW(0x04E00400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subqx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04E00400 | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlqx_ri(XG, IS)                                                    \
        shlqx3ri(W(XG), W(XG), W(IS))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05E03800 | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x04D38000 | MXM(REG(XG), TmmM,    0x00))

#define shlqx3ri(XD, XS, IT)                                                \
        EMITW(0x04A09C00 | MXM(REG(XD), REG(XS), 0x00) |                    \
                        (0x20 & VAL(IT)) << 17 | (0x1F & VAL(IT)) << 16)

#define shlqx3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        shlqx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrqx_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shrqx3ri(W(XG), W(XG), W(IS))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05E03800 | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x04D18000 | MXM(REG(XG), TmmM,    0x00))

#define shrqx3ri(XD, XS, IT)                                                \
        EMITW(0x04A09400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000800) | (M(VAL(IT) != 0) & 0x00000000) |   \
                        (0x20 &-VAL(IT)) << 17 | (0x1F &-VAL(IT)) << 16)

#define shrqx3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        shrqx_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrqn_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shrqn3ri(W(XG), W(XG), W(IS))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05E03800 | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x04D08000 | MXM(REG(XG), TmmM,    0x00))

#define shrqn3ri(XD, XS, IT)                                                \
        EMITW(0x04A09000 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000C00) | (M(VAL(IT) != 0) & 0x00000000) |   \
                        (0x20 &-VAL(IT)) << 17 | (0x1F &-VAL(IT)) << 16)

#define shrqn3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        shrqn_ld(W(XD), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04D38000 | MXM(REG(XG), REG(XS), 0x00))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x04D38000 | MXM(REG(XG), TmmM,    0x00))

#define svlqx3rr(XD, XS, XT)                                                \
        movqx_rr(W(XD), W(XS))                                              \
        svlqx_rr(W(XD), W(XT))

#define svlqx3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        svlqx_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04D18000 | MXM(REG(XG), REG(XS), 0x00))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x04D18000 | MXM(REG(XG), TmmM,    0x00))

#define svrqx3rr(XD, XS, XT)                                                \
        movqx_rr(W(XD), W(XS))                                              \
        svrqx_rr(W(XD), W(XT))

#define svrqx3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        svrqx_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04D08000 | MXM(REG(XG), REG(XS), 0x00))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x04D08000 | MXM(REG(XG), TmmM,    0x00))

#define svrqn3rr(XD, XS, XT)                                                \
        movqx_rr(W(XD), W(XS))                                              \
        svrqn_rr(W(XD), W(XT))

#define svrqn3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        svrqn_ld(W(XD), W(MT), W(DT))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_SVEX1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A64_SVEX1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

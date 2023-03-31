/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_128X2V1_H
#define RT_RTARCH_A64_128X2V1_H

#include "rtarch_a32_128x2v1.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64_128x2v1.h: Implementation of AArch64 fp64 NEON instruction pairs.
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

#if (RT_128X2 != 0) && (RT_SIMD_COMPAT_XMM > 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmdx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmjx_st(W(XS), W(MD), W(DD))

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movdx_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movdx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x3DC00000 | MPM(RYG(XD), MOD(MS), VYL(DS), B4(DS), L2(DS)))

#define movdx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DD), B4(DD), L2(DD)))  \
        EMITW(0x3D800000 | MPM(RYG(XS), MOD(MD), VYL(DD), B4(DD), L2(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvdx_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x6EA01C00 | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvdx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EA01C00 | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvdx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VAL(DG), B4(DG), L2(DG)))  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VYL(DG), B4(DG), L2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define anddx_rr(XG, XS)                                                    \
        anddx3rr(W(XG), W(XG), W(XS))

#define anddx_ld(XG, MS, DS)                                                \
        anddx3ld(W(XG), W(XG), W(MS), W(DS))

#define anddx3rr(XD, XS, XT)                                                \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E201C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define anddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E201C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anndx_rr(XG, XS)                                                    \
        anndx3rr(W(XG), W(XG), W(XS))

#define anndx_ld(XG, MS, DS)                                                \
        anndx3ld(W(XG), W(XG), W(MS), W(DS))

#define anndx3rr(XD, XS, XT)                                                \
        EMITW(0x4E601C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4E601C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define anndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E601C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E601C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrdx_rr(XG, XS)                                                    \
        orrdx3rr(W(XG), W(XG), W(XS))

#define orrdx_ld(XG, MS, DS)                                                \
        orrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrdx3rr(XD, XS, XT)                                                \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orndx_rr(XG, XS)                                                    \
        orndx3rr(W(XG), W(XG), W(XS))

#define orndx_ld(XG, MS, DS)                                                \
        orndx3ld(W(XG), W(XG), W(MS), W(DS))

#define orndx3rr(XD, XS, XT)                                                \
        EMITW(0x4EE01C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4EE01C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define orndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE01C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE01C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xordx_rr(XG, XS)                                                    \
        xordx3rr(W(XG), W(XG), W(XS))

#define xordx_ld(XG, MS, DS)                                                \
        xordx3ld(W(XG), W(XG), W(MS), W(DS))

#define xordx3rr(XD, XS, XT)                                                \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E201C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xordx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E201C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notdx_rx(XG)                                                        \
        notdx_rr(W(XG), W(XG))

#define notdx_rr(XD, XS)                                                    \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XS), 0x00))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negds_rx(XG)                                                        \
        negds_rr(W(XG), W(XG))

#define negds_rr(XD, XS)                                                    \
        EMITW(0x6EE0F800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE0F800 | MXM(RYG(XD), RYG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addds_rr(XG, XS)                                                    \
        addds3rr(W(XG), W(XG), W(XS))

#define addds_ld(XG, MS, DS)                                                \
        addds3ld(W(XG), W(XG), W(MS), W(DS))

#define addds3rr(XD, XS, XT)                                                \
        EMITW(0x4E60D400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E60D400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60D400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60D400 | MXM(RYG(XD), RYG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  adpds3rr
#define adpds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60D400 | MXM(REG(XD), REG(XS), RYG(XS)))                  \
        EMITW(0x6E60D400 | MXM(RYG(XD), REG(XT), RYG(XT)))

#undef  adpds3ld
#define adpds3ld(XD, XS, MT, DT)                                            \
        EMITW(0x6E60D400 | MXM(REG(XD), REG(XS), RYG(XS)))                  \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(RYG(XD), MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60D400 | MXM(RYG(XD), RYG(XD), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subds_rr(XG, XS)                                                    \
        subds3rr(W(XG), W(XG), W(XS))

#define subds_ld(XG, MS, DS)                                                \
        subds3ld(W(XG), W(XG), W(MS), W(DS))

#define subds3rr(XD, XS, XT)                                                \
        EMITW(0x4EE0D400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE0D400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE0D400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE0D400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulds_rr(XG, XS)                                                    \
        mulds3rr(W(XG), W(XG), W(XS))

#define mulds_ld(XG, MS, DS)                                                \
        mulds3ld(W(XG), W(XG), W(MS), W(DS))

#define mulds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60DC00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60DC00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60DC00 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divds_rr(XG, XS)                                                    \
        divds3rr(W(XG), W(XG), W(XS))

#define divds_ld(XG, MS, DS)                                                \
        divds3ld(W(XG), W(XG), W(MS), W(DS))

#define divds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60FC00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E60FC00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60FC00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60FC00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrds_rr(XD, XS)                                                    \
        EMITW(0x6EE1F800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE1F800 | MXM(RYG(XD), RYG(XS), 0x00))

#define sqrds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EE1F800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EE1F800 | MXM(RYG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceds_rr(XD, XS)                                                    \
        EMITW(0x4EE1D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rcsds_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E60FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x4E60FC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseds_rr(XD, XS)                                                    \
        EMITW(0x6EE1D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE1D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rssds_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E60DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EE0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x4EE0FC00 | MXM(RYG(XS), RYG(XS), RYG(XG)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmads_rr(XG, XS, XT)                                                \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4E60CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmads_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsds_rr(XG, XS, XT)                                                \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4EE0CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE0CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minds_rr(XG, XS)                                                    \
        minds3rr(W(XG), W(XG), W(XS))

#define minds_ld(XG, MS, DS)                                                \
        minds3ld(W(XG), W(XG), W(MS), W(DS))

#define minds3rr(XD, XS, XT)                                                \
        EMITW(0x4EE0F400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE0F400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE0F400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE0F400 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxds_rr(XG, XS)                                                    \
        maxds3rr(W(XG), W(XG), W(XS))

#define maxds_ld(XG, MS, DS)                                                \
        maxds3ld(W(XG), W(XG), W(MS), W(DS))

#define maxds3rr(XD, XS, XT)                                                \
        EMITW(0x4E60F400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E60F400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60F400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60F400 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqds_rr(XG, XS)                                                    \
        ceqds3rr(W(XG), W(XG), W(XS))

#define ceqds_ld(XG, MS, DS)                                                \
        ceqds3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqds3rr(XD, XS, XT)                                                \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E60E400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60E400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneds_rr(XG, XS)                                                    \
        cneds3rr(W(XG), W(XG), W(XS))

#define cneds_ld(XG, MS, DS)                                                \
        cneds3ld(W(XG), W(XG), W(MS), W(DS))

#define cneds3rr(XD, XS, XT)                                                \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x4E60E400 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

#define cneds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E60E400 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltds_rr(XG, XS)                                                    \
        cltds3rr(W(XG), W(XG), W(XS))

#define cltds_ld(XG, MS, DS)                                                \
        cltds3ld(W(XG), W(XG), W(MS), W(DS))

#define cltds3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0E400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleds_rr(XG, XS)                                                    \
        cleds3rr(W(XG), W(XG), W(XS))

#define cleds_ld(XG, MS, DS)                                                \
        cleds3ld(W(XG), W(XG), W(MS), W(DS))

#define cleds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6E60E400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60E400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60E400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtds_rr(XG, XS)                                                    \
        cgtds3rr(W(XG), W(XG), W(XS))

#define cgtds_ld(XG, MS, DS)                                                \
        cgtds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtds3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeds_rr(XG, XS)                                                    \
        cgeds3rr(W(XG), W(XG), W(XS))

#define cgeds_ld(XG, MS, DS)                                                \
        cgeds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E60E400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6E60E400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_256     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_256     0x04    /*  all satisfy the condition */

#define mkjdx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4E201C00 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                                (0x04 - RT_SIMD_MASK_##mask##64_256) << 21) \
        EMITW(0x4EB1B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwxZri(Reax, IB(RT_SIMD_MASK_##mask##64_256))                     \
        jezxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE19800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE19800 | MXM(RYG(XD), TmmM,    0x00))

#define cvzds_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE1B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE1B800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE1B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE18800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE18800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE18800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE18800 | MXM(RYG(XD), TmmM,    0x00))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE1A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE1A800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4EE1A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E619800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E619800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E619800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E619800 | MXM(RYG(XD), TmmM,    0x00))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E61B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E61B800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E61B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnds_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E618800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E618800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E618800 | MXM(RYG(XD), TmmM,    0x00))

#define cvnds_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E61A800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E61A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndn_rr(XD, XS)     /* round towards near */                       \
        cvtdn_rr(W(XD), W(XS))

#define cvndn_ld(XD, MS, DS) /* round towards near */                       \
        cvtdn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
        EMITW(0x6EE19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rndds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EE19800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x6EE19800 | MXM(RYG(XD), TmmM,    0x00))

#define cvtds_rr(XD, XS)                                                    \
        rndds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvtds_ld(XD, MS, DS)                                                \
        rndds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtdn_rr(XD, XS)                                                    \
        EMITW(0x4E61D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtdn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E61D800 | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B4(DS), L2(DS)))  \
        EMITW(0x4E61D800 | MXM(RYG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp64 SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrds_rr(XD, XS, mode)                                              \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E618800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvrds_rr(XD, XS, mode)                                              \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)      \
        EMITW(0x4E61A800 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

/************   packed double-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define adddx_rr(XG, XS)                                                    \
        adddx3rr(W(XG), W(XG), W(XS))

#define adddx_ld(XG, MS, DS)                                                \
        adddx3ld(W(XG), W(XG), W(MS), W(DS))

#define adddx3rr(XD, XS, XT)                                                \
        EMITW(0x4EE08400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE08400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE08400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE08400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subdx_rr(XG, XS)                                                    \
        subdx3rr(W(XG), W(XG), W(XS))

#define subdx_ld(XG, MS, DS)                                                \
        subdx3ld(W(XG), W(XG), W(MS), W(DS))

#define subdx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE08400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE08400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE08400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE08400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define muldx_rr(XG, XS)                                                    \
        muldx3rr(W(XG), W(XG), W(XS))

#define muldx_ld(XG, MS, DS)                                                \
        muldx3ld(W(XG), W(XG), W(MS), W(DS))

#define muldx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

#define muldx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movzx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        stack_ld(Recx)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shldx_ri(XG, IS)     /* emits shift-right with out-of-range args */ \
        shldx3ri(W(XG), W(XG), W(IS))

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shldx3ld(W(XG), W(XG), W(MS), W(DS))

#define shldx3ri(XD, XS, IT)                                                \
        EMITW(0x4F400400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) < 64) & 0x00005000) | (M(VAL(IT) > 63) & 0x20000000) |   \
        (M(VAL(IT) < 64) & ((0x3F & VAL(IT)) << 16)))                       \
        EMITW(0x4F400400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) < 64) & 0x00005000) | (M(VAL(IT) > 63) & 0x20000000) |   \
        (M(VAL(IT) < 64) & ((0x3F & VAL(IT)) << 16)))

#define shldx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdx_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrdx3ri(W(XG), W(XG), W(IS))

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdx3ri(XD, XS, IT)                                                \
        EMITW(0x4F400400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 64) & ((0x3F &-VAL(IT)) << 16)))                       \
        EMITW(0x4F400400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x20000000) |   \
        (M(VAL(IT) < 64) & ((0x3F &-VAL(IT)) << 16)))

#define shrdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrdn_ri(XG, IS)     /* emits shift-left for immediate-zero args */ \
        shrdn3ri(W(XG), W(XG), W(IS))

#define shrdn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrdn3ri(XD, XS, IT)                                                \
        EMITW(0x4F400400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 64) & ((0x3F &-VAL(IT)) << 16)))                       \
        EMITW(0x4F400400 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00005000) | (M(VAL(IT) != 0) & 0x00000000) |   \
        (M(VAL(IT) < 64) & ((0x3F &-VAL(IT)) << 16)))

#define shrdn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x4EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svldx3rr(W(XG), W(XG), W(XS))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svldx3ld(W(XG), W(XG), W(MS), W(DS))

#define svldx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE04400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svldx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdx3rr(W(XG), W(XG), W(XS))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    RYG(XT), 0x00))                     \
        EMITW(0x6EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

#define svrdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrdn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrdn3rr(W(XG), W(XG), W(XS))

#define svrdn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrdn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrdn3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XT), 0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    RYG(XT), 0x00))                     \
        EMITW(0x4EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

#define svrdn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(RYG(XD), RYG(XS), TmmM))

/****************   packed double-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mindx_rr(XG, XS)                                                    \
        mindx3rr(W(XG), W(XG), W(XS))

#define mindx_ld(XG, MS, DS)                                                \
        mindx3ld(W(XG), W(XG), W(MS), W(DS))

#define mindx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

#define mindx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x54000042)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mindn_rr(XG, XS)                                                    \
        mindn3rr(W(XG), W(XG), W(XS))

#define mindn_ld(XG, MS, DS)                                                \
        mindn3ld(W(XG), W(XG), W(MS), W(DS))

#define mindn3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

#define mindn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x5400004A)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxdx_rr(XG, XS)                                                    \
        maxdx3rr(W(XG), W(XG), W(XS))

#define maxdx_ld(XG, MS, DS)                                                \
        maxdx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxdx3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

#define maxdx3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x54000049)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxdn_rr(XG, XS)                                                    \
        maxdn3rr(W(XG), W(XG), W(XS))

#define maxdn_ld(XG, MS, DS)                                                \
        maxdn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxdn3rr(XD, XS, XT)                                                \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

#define maxdn3ld(XD, XS, MT, DT)                                            \
        movdx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movdx_ld(W(XD), W(MT), W(DT))                                       \
        movdx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x00))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x00))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x08))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x08))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x10))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x10))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR01(0x18))                              \
        cmpzx_rm(Reax,  Mebp, inf_SCR02(0x18))                              \
        EMITW(0x5400004D)                                                   \
        movzx_st(Reax,  Mebp, inf_SCR02(0x18))                              \
        stack_ld(Reax)                                                      \
        movdx_ld(W(XD), Mebp, inf_SCR02(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqdx_rr(XG, XS)                                                    \
        ceqdx3rr(W(XG), W(XG), W(XS))

#define ceqdx_ld(XG, MS, DS)                                                \
        ceqdx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqdx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE08C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE08C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE08C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE08C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnedx_rr(XG, XS)                                                    \
        cnedx3rr(W(XG), W(XG), W(XS))

#define cnedx_ld(XG, MS, DS)                                                \
        cnedx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnedx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE08C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x6EE08C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

#define cnedx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE08C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE08C00 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltdx_rr(XG, XS)                                                    \
        cltdx3rr(W(XG), W(XG), W(XS))

#define cltdx_ld(XG, MS, DS)                                                \
        cltdx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltdx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE03400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6EE03400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltdn_rr(XG, XS)                                                    \
        cltdn3rr(W(XG), W(XG), W(XS))

#define cltdn_ld(XG, MS, DS)                                                \
        cltdn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltdn3rr(XD, XS, XT)                                                \
        EMITW(0x4EE03400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4EE03400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltdn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cledx_rr(XG, XS)                                                    \
        cledx3rr(W(XG), W(XG), W(XS))

#define cledx_ld(XG, MS, DS)                                                \
        cledx3ld(W(XG), W(XG), W(MS), W(DS))

#define cledx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE03C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6EE03C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cledx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define cledn_rr(XG, XS)                                                    \
        cledn3rr(W(XG), W(XG), W(XS))

#define cledn_ld(XG, MS, DS)                                                \
        cledn3ld(W(XG), W(XG), W(MS), W(DS))

#define cledn3rr(XD, XS, XT)                                                \
        EMITW(0x4EE03C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4EE03C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cledn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtdx_rr(XG, XS)                                                    \
        cgtdx3rr(W(XG), W(XG), W(XS))

#define cgtdx_ld(XG, MS, DS)                                                \
        cgtdx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtdx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE03400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE03400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtdn_rr(XG, XS)                                                    \
        cgtdn3rr(W(XG), W(XG), W(XS))

#define cgtdn_ld(XG, MS, DS)                                                \
        cgtdn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtdn3rr(XD, XS, XT)                                                \
        EMITW(0x4EE03400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE03400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtdn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgedx_rr(XG, XS)                                                    \
        cgedx3rr(W(XG), W(XG), W(XS))

#define cgedx_ld(XG, MS, DS)                                                \
        cgedx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgedx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE03C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE03C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgedx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x6EE03C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgedn_rr(XG, XS)                                                    \
        cgedn3rr(W(XG), W(XG), W(XS))

#define cgedn_ld(XG, MS, DS)                                                \
        cgedn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgedn3rr(XD, XS, XT)                                                \
        EMITW(0x4EE03C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE03C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgedn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B4(DT), L2(DT)))  \
        EMITW(0x4EE03C00 | MXM(RYG(XD), RYG(XS), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A64_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

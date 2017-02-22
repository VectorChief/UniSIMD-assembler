/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_256V1_H
#define RT_RTARCH_A64_256V1_H

#include "rtarch_a32_256v1.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64_256v1.h: Implementation of AArch64 fp64 NEON instructions (pairs).
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

#if defined (RT_256) && (RT_256 != 0) && (RT_SIMD_COMPAT_XMM > 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movdx_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movdx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(RYG(XD), MOD(MS), VYL(DS), B2(DS), P2(DS)))

#define movdx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VYL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(RYG(XS), MOD(MD), VYL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvdx_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x6EA01C00 | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvdx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvdx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VYL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VYL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VYL(DG), B2(DG), P2(DG)))

/* and (G = G & S), (D = S & T) if (D != S) */

#define anddx_rr(XG, XS)                                                    \
        anddx3rr(W(XG), W(XG), W(XS))

#define anddx_ld(XG, MS, DS)                                                \
        anddx3ld(W(XG), W(XG), W(MS), W(DS))

#define anddx3rr(XD, XS, XT)                                                \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E201C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define anddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E201C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (D != S) */

#define anndx_rr(XG, XS)                                                    \
        anndx3rr(W(XG), W(XG), W(XS))

#define anndx_ld(XG, MS, DS)                                                \
        anndx3ld(W(XG), W(XG), W(MS), W(DS))

#define anndx3rr(XD, XS, XT)                                                \
        EMITW(0x4E601C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4E601C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define anndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E601C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E601C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (D != S) */

#define orrdx_rr(XG, XS)                                                    \
        orrdx3rr(W(XG), W(XG), W(XS))

#define orrdx_ld(XG, MS, DS)                                                \
        orrdx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrdx3rr(XD, XS, XT)                                                \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA01C00 | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (D != S) */

#define orndx_rr(XG, XS)                                                    \
        orndx3rr(W(XG), W(XG), W(XS))

#define orndx_ld(XG, MS, DS)                                                \
        orndx3ld(W(XG), W(XG), W(MS), W(DS))

#define orndx3rr(XD, XS, XT)                                                \
        EMITW(0x4EE01C00 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x4EE01C00 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define orndx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE01C00 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE01C00 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (D != S) */

#define xordx_rr(XG, XS)                                                    \
        xordx3rr(W(XG), W(XG), W(XS))

#define xordx_ld(XG, MS, DS)                                                \
        xordx3ld(W(XG), W(XG), W(MS), W(DS))

#define xordx3rr(XD, XS, XT)                                                \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E201C00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xordx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
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

/* add (G = G + S), (D = S + T) if (D != S) */

#define addds_rr(XG, XS)                                                    \
        addds3rr(W(XG), W(XG), W(XS))

#define addds_ld(XG, MS, DS)                                                \
        addds3ld(W(XG), W(XG), W(MS), W(DS))

#define addds3rr(XD, XS, XT)                                                \
        EMITW(0x4E60D400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E60D400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60D400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60D400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subds_rr(XG, XS)                                                    \
        subds3rr(W(XG), W(XG), W(XS))

#define subds_ld(XG, MS, DS)                                                \
        subds3ld(W(XG), W(XG), W(MS), W(DS))

#define subds3rr(XD, XS, XT)                                                \
        EMITW(0x4EE0D400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE0D400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0D400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0D400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (D != S) */

#define mulds_rr(XG, XS)                                                    \
        mulds3rr(W(XG), W(XG), W(XS))

#define mulds_ld(XG, MS, DS)                                                \
        mulds3ld(W(XG), W(XG), W(MS), W(DS))

#define mulds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60DC00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E60DC00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60DC00 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60DC00 | MXM(RYG(XD), RYG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (D != S) */

#define divds_rr(XG, XS)                                                    \
        divds3rr(W(XG), W(XG), W(XS))

#define divds_ld(XG, MS, DS)                                                \
        divds3ld(W(XG), W(XG), W(MS), W(DS))

#define divds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60FC00 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E60FC00 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60FC00 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60FC00 | MXM(RYG(XD), RYG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrds_rr(XD, XS)                                                    \
        EMITW(0x6EE1F800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE1F800 | MXM(RYG(XD), RYG(XS), 0x00))

#define sqrds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE1F800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE1F800 | MXM(RYG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
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

        /* rcp defined in rtbase.h
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

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmads_rr(XG, XS, XT)                                                \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4E60CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmads_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsds_rr(XG, XS, XT)                                                \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x4EE0CC00 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsds_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0CC00 | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0CC00 | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (D != S) */

#define minds_rr(XG, XS)                                                    \
        minds3rr(W(XG), W(XG), W(XS))

#define minds_ld(XG, MS, DS)                                                \
        minds3ld(W(XG), W(XG), W(MS), W(DS))

#define minds3rr(XD, XS, XT)                                                \
        EMITW(0x4EE0F400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE0F400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0F400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE0F400 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (D != S) */

#define maxds_rr(XG, XS)                                                    \
        maxds3rr(W(XG), W(XG), W(XS))

#define maxds_ld(XG, MS, DS)                                                \
        maxds3ld(W(XG), W(XG), W(MS), W(DS))

#define maxds3rr(XD, XS, XT)                                                \
        EMITW(0x4E60F400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E60F400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60F400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60F400 | MXM(RYG(XD), RYG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (D != S) */

#define ceqds_rr(XG, XS)                                                    \
        ceqds3rr(W(XG), W(XG), W(XS))

#define ceqds_ld(XG, MS, DS)                                                \
        ceqds3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqds3rr(XD, XS, XT)                                                \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4E60E400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60E400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (D != S) */

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
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E60E400 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(RYG(XD), RYG(XD), 0x00))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (D != S) */

#define cltds_rr(XG, XS)                                                    \
        cltds3rr(W(XG), W(XG), W(XS))

#define cltds_ld(XG, MS, DS)                                                \
        cltds3ld(W(XG), W(XG), W(MS), W(DS))

#define cltds3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0E400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (D != S) */

#define cleds_rr(XG, XS)                                                    \
        cleds3rr(W(XG), W(XG), W(XS))

#define cleds_ld(XG, MS, DS)                                                \
        cleds3ld(W(XG), W(XG), W(MS), W(DS))

#define cleds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x6E60E400 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60E400 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60E400 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (D != S) */

#define cgtds_rr(XG, XS)                                                    \
        cgtds3rr(W(XG), W(XG), W(XS))

#define cgtds_ld(XG, MS, DS)                                                \
        cgtds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtds3rr(XD, XS, XT)                                                \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE0E400 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (D != S) */

#define cgeds_rr(XG, XS)                                                    \
        cgeds3rr(W(XG), W(XG), W(XS))

#define cgeds_ld(XG, MS, DS)                                                \
        cgeds3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeds3rr(XD, XS, XT)                                                \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E60E400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeds3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E60E400 | MXM(RYG(XD), RYG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE64_256     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_256     0x04    /*  all satisfy the condition */

#define mkjdx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4E201C00 | MXM(TmmM,    REG(XS), RYG(XS)) |                 \
                                (0x04 - RT_SIMD_MASK_##mask##64_256) << 21) \
        EMITW(0x4EB1B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask##64_256))                     \
        jezxx_lb(lb)

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzds_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE19800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE19800 | MXM(RYG(XD), TmmM,    0x00))

#define cvzds_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EE1B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvzds_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1B800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpds_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE18800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE18800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE18800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE18800 | MXM(RYG(XD), TmmM,    0x00))

#define cvpds_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EE1A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4EE1A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvpds_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1A800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EE1A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmds_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E619800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E619800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E619800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E619800 | MXM(RYG(XD), TmmM,    0x00))

#define cvmds_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E61B800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61B800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvmds_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61B800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61B800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnds_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E618800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E618800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rnnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E618800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E618800 | MXM(RYG(XD), TmmM,    0x00))

#define cvnds_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E61A800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61A800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvnds_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61A800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61A800 | MXM(RYG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvndn_rr(XD, XS)     /* round towards near */                       \
        cvtdn_rr(W(XD), W(XS))

#define cvndn_ld(XD, MS, DS) /* round towards near */                       \
        cvtdn_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndds_rr(XD, XS)                                                    \
        EMITW(0x6EE19800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x6EE19800 | MXM(RYG(XD), RYG(XS), 0x00))

#define rndds_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE19800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE19800 | MXM(RYG(XD), TmmM,    0x00))

#define cvtds_rr(XD, XS)                                                    \
        rndds_rr(W(XD), W(XS))                                              \
        cvzds_rr(W(XD), W(XD))

#define cvtds_ld(XD, MS, DS)                                                \
        rndds_ld(W(XD), W(MS), W(DS))                                       \
        cvzds_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtdn_rr(XD, XS)                                                    \
        EMITW(0x4E61D800 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x4E61D800 | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtdn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61D800 | MXM(REG(XD), TmmM,    0x00))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E61D800 | MXM(RYG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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

/* add (G = G + S), (D = S + T) if (D != S) */

#define adddx_rr(XG, XS)                                                    \
        adddx3rr(W(XG), W(XG), W(XS))

#define adddx_ld(XG, MS, DS)                                                \
        adddx3ld(W(XG), W(XG), W(MS), W(DS))

#define adddx3rr(XD, XS, XT)                                                \
        EMITW(0x4EE08400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x4EE08400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define adddx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE08400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE08400 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subdx_rr(XG, XS)                                                    \
        subdx3rr(W(XG), W(XG), W(XS))

#define subdx_ld(XG, MS, DS)                                                \
        subdx3ld(W(XG), W(XG), W(MS), W(DS))

#define subdx3rr(XD, XS, XT)                                                \
        EMITW(0x6EE08400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6EE08400 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subdx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE08400 | MXM(REG(XD), REG(XS), TmmM))                     \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EE08400 | MXM(RYG(XD), RYG(XS), TmmM))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shldx_ri(XG, IS)                                                    \
        EMITW(0x4F405400 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)    \
        EMITW(0x4F405400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)

#define shldx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svldx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define svldx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrdx_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)    \
        EMITW(0x4F400400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)

#define shrdx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrdx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    RYG(XS), 0x00))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrdx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE04400 | MXM(RYG(XG), RYG(XG), TmmM))


#define shrdn_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F400400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)    \
        EMITW(0x4F400400 | MXM(RYG(XG), RYG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x3F &-VAL(IS)) << 16)

#define shrdn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E080400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x4EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrdn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EE0B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6EE0B800 | MXM(TmmM,    RYG(XS), 0x00))                     \
        EMITW(0x4EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

#define svrdn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(REG(XG), REG(XG), TmmM))                     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EE0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EE04400 | MXM(RYG(XG), RYG(XG), TmmM))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A64_256V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

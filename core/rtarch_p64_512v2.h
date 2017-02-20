/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P64_512V2_H
#define RT_RTARCH_P64_512V2_H

#include "rtarch_p32_512v2.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p64_512v2.h: Implementation of Power fp64 VSX1/2 instructions (quads).
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

#if defined (RT_512) && (RT_512 != 0) && (RT_SIMD_COMPAT_XMM > 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/***************   packed double-precision generic move/logic   ***************/

/* mov (D = S) */

#define movqx_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000490 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000490 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movqx_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(RYG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000698 | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000698 | MXM(RYG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movqx_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000799 | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VYL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VYL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000799 | MXM(RYG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VXL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VXL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000798 | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VZL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VZL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000798 | MXM(RYG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvqx_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000030 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000430 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mmvqx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000032 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000432 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */

#define mmvqx_st(XS, MG, DG)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000799 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VYL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VYL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF000043F | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x7C000799 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VXL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VXL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF0000035 | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000799 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VZL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VZL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF0000435 | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x7C000799 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* and (G = G & S), (D = S & T) if (D != S) */

#define andqx_rr(XG, XS)                                                    \
        andqx3rr(W(XG), W(XG), W(XS))

#define andqx_ld(XG, MS, DS)                                                \
        andqx3ld(W(XG), W(XG), W(MS), W(DS))

#define andqx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000410 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000410 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andqx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000412 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000412 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* ann (G = ~G & S), (D = ~S & T) if (D != S) */

#define annqx_rr(XG, XS)                                                    \
        annqx3rr(W(XG), W(XG), W(XS))

#define annqx_ld(XG, MS, DS)                                                \
        annqx3ld(W(XG), W(XG), W(MS), W(DS))

#define annqx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000457 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000457 | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000450 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000450 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define annqx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000454 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000454 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (D != S) */

#define orrqx_rr(XG, XS)                                                    \
        orrqx3rr(W(XG), W(XG), W(XS))

#define orrqx_ld(XG, MS, DS)                                                \
        orrqx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrqx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000490 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000490 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrqx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000492 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000492 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* orn (G = ~G | S), (D = ~S | T) if (D != S) */

#if (RT_512 < 2)

#define ornqx_rr(XG, XS)                                                    \
        notqx_rx(W(XG))                                                     \
        orrqx_rr(W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        notqx_rx(W(XG))                                                     \
        orrqx_ld(W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        movqx_rr(W(XD), W(XS))                                              \
        ornqx_rr(W(XD), W(XT))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        movqx_rr(W(XD), W(XS))                                              \
        ornqx_ld(W(XD), W(MT), W(DT))

#else /* RT_512 >= 2 */

#define ornqx_rr(XG, XS)                                                    \
        ornqx3rr(W(XG), W(XG), W(XS))

#define ornqx_ld(XG, MS, DS)                                                \
        ornqx3ld(W(XG), W(XG), W(MS), W(DS))

#define ornqx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000557 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000557 | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000550 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000550 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define ornqx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000554 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000554 | MXM(RYG(XD), TmmM,    RYG(XS)))

#endif /* RT_512 >= 2 */

/* xor (G = G ^ S), (D = S ^ T) if (D != S) */

#define xorqx_rr(XG, XS)                                                    \
        xorqx3rr(W(XG), W(XG), W(XS))

#define xorqx_ld(XG, MS, DS)                                                \
        xorqx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorqx3rr(XD, XS, XT)                                                \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF00004D0 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00004D0 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xorqx3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00004D2 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00004D2 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* not (G = ~G) */

#define notqx_rx(XG)                                                        \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        EMITW(0xF0000517 | MXM(RYG(XG), RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000510 | MXM(REG(XG), REG(XG), REG(XG)))                  \
        EMITW(0xF0000510 | MXM(RYG(XG), RYG(XG), RYG(XG)))

/************   packed double-precision floating-point arithmetic   ***********/

/* neg (G = -G) */

#define negqs_rx(XG)                                                        \
        EMITW(0xF00007E7 | MXM(REG(XG), 0x00,    REG(XG)))                  \
        EMITW(0xF00007E7 | MXM(RYG(XG), 0x00,    RYG(XG)))                  \
        EMITW(0xF00007E4 | MXM(REG(XG), 0x00,    REG(XG)))                  \
        EMITW(0xF00007E4 | MXM(RYG(XG), 0x00,    RYG(XG)))

/* add (G = G + S), (D = S + T) if (D != S) */

#define addqs_rr(XG, XS)                                                    \
        addqs3rr(W(XG), W(XG), W(XS))

#define addqs_ld(XG, MS, DS)                                                \
        addqs3ld(W(XG), W(XG), W(MS), W(DS))

#define addqs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000307 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000307 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000300 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000300 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000307 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000307 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000302 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000302 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subqs_rr(XG, XS)                                                    \
        subqs3rr(W(XG), W(XG), W(XS))

#define subqs_ld(XG, MS, DS)                                                \
        subqs3ld(W(XG), W(XG), W(MS), W(DS))

#define subqs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000347 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000347 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000340 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000340 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000347 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000347 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000342 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000342 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* mul (G = G * S), (D = S * T) if (D != S) */

#define mulqs_rr(XG, XS)                                                    \
        mulqs3rr(W(XG), W(XG), W(XS))

#define mulqs_ld(XG, MS, DS)                                                \
        mulqs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulqs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000387 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000387 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000380 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000380 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000387 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000387 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000382 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000382 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* div (G = G / S), (D = S / T) if (D != S) */

#define divqs_rr(XG, XS)                                                    \
        divqs3rr(W(XG), W(XG), W(XS))

#define divqs_ld(XG, MS, DS)                                                \
        divqs3ld(W(XG), W(XG), W(MS), W(DS))

#define divqs3rr(XD, XS, XT)                                                \
        EMITW(0xF00003C7 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00003C7 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF00003C0 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00003C0 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00003C7 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00003C7 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00003C2 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF00003C2 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* sqr (D = sqrt S) */

#define sqrqs_rr(XD, XS)                                                    \
        EMITW(0xF000032F | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000032F | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF000032C | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000032C | MXM(RYG(XD), 0x00,    RYG(XS)))

#define sqrqs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000032F | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000032F | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000032E | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000032E | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceqs_rr(XD, XS)                                                    \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsqs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseqs_rr(XD, XS)                                                    \
        sqrqs_rr(W(XD), W(XS))                                              \
        movqx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movqx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divqs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssqs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaqs_rr(XG, XS, XT)                                                \
        EMITW(0xF000030F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000030F | MXM(RYG(XG), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000308 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF0000308 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmaqs_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000030F | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000030F | MXM(RYG(XG), RYG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000030A | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000030A | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsqs_rr(XG, XS, XT)                                                \
        EMITW(0xF000078F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000078F | MXM(RYG(XG), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000788 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF0000788 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsqs_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000078F | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000078F | MXM(RYG(XG), RYG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000078A | MXM(REG(XG), REG(XS), TmmM))                     \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000078A | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed double-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (D != S) */

#define minqs_rr(XG, XS)                                                    \
        minqs3rr(W(XG), W(XG), W(XS))

#define minqs_ld(XG, MS, DS)                                                \
        minqs3ld(W(XG), W(XG), W(MS), W(DS))

#define minqs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000747 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000747 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000740 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000740 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000747 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000747 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000742 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000742 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (D != S) */

#define maxqs_rr(XG, XS)                                                    \
        maxqs3rr(W(XG), W(XG), W(XS))

#define maxqs_ld(XG, MS, DS)                                                \
        maxqs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxqs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000707 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000707 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000700 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000700 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000707 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000707 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000702 | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF0000702 | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/* cmp (G = G ? S), (D = S ? T) if (D != S) */

#define ceqqs_rr(XG, XS)                                                    \
        ceqqs3rr(W(XG), W(XG), W(XS))

#define ceqqs_ld(XG, MS, DS)                                                \
        ceqqs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqqs3rr(XD, XS, XT)                                                \
        EMITW(0xF000031F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000031F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000318 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000318 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031A | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */


#define cneqs_rr(XG, XS)                                                    \
        cneqs3rr(W(XG), W(XG), W(XS))

#define cneqs_ld(XG, MS, DS)                                                \
        cneqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cneqs3rr(XD, XS, XT)                                                \
        EMITW(0xF000031F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0xF000031F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0xF0000318 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000510 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0xF0000318 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000510 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define cneqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000510 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000031A | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000510 | MXM(RYG(XD), RYG(XD), RYG(XD)))


#define cltqs_rr(XG, XS)                                                    \
        cltqs3rr(W(XG), W(XG), W(XS))

#define cltqs_ld(XG, MS, DS)                                                \
        cltqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltqs3rr(XD, XS, XT)                                                \
        EMITW(0xF000035F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF000035F | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000358 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000358 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035C | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035C | MXM(RYG(XD), TmmM,    RYG(XS)))


#define cleqs_rr(XG, XS)                                                    \
        cleqs3rr(W(XG), W(XG), W(XS))

#define cleqs_ld(XG, MS, DS)                                                \
        cleqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cleqs3rr(XD, XS, XT)                                                \
        EMITW(0xF000039F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF000039F | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000398 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000398 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039C | MXM(REG(XD), TmmM,    REG(XS)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039C | MXM(RYG(XD), TmmM,    RYG(XS)))


#define cgtqs_rr(XG, XS)                                                    \
        cgtqs3rr(W(XG), W(XG), W(XS))

#define cgtqs_ld(XG, MS, DS)                                                \
        cgtqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtqs3rr(XD, XS, XT)                                                \
        EMITW(0xF000035F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000035F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000358 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000358 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000035A | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */


#define cgeqs_rr(XG, XS)                                                    \
        cgeqs3rr(W(XG), W(XG), W(XS))

#define cgeqs_ld(XG, MS, DS)                                                \
        cgeqs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeqs3rr(XD, XS, XT)                                                \
        EMITW(0xF000039F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000039F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000398 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000398 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeqs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VYL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VYL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VXL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VXL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039A | MXM(REG(XD), REG(XS), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VZL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VZL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000039A | MXM(RYG(XD), RYG(XS), TmmM))/* ^ == -1 if true */

/*************   packed double-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000367 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000367 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000364 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000364 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnzqs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000367 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000367 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000366 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000366 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvzqs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000763 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000763 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000760 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000760 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvzqs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000763 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000763 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000762 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000762 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpqs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00003A7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003A7 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00003A4 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003A4 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnpqs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003A7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003A7 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003A6 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003A6 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

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
        EMITW(0xF00003E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003E7 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00003E4 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003E4 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnmqs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003E7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003E7 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003E6 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003E6 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

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
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003AF | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00003AC | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003AC | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnnqs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AE | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AE | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

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

/************   packed double-precision integer arithmetic/shifts   ***********/

#if (RT_512 < 2)

/* add (G = G + S) */

#define addqx_rr(XG, XS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x10))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x18))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x18))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x20))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x20))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x28))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x28))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x30))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x30))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x38))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x38))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define addqx_ld(XG, MS, DS)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(MS), W(DS))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x10))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x18))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x18))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x20))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x20))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x28))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x28))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x30))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x30))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x38))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x38))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* sub (G = G - S) */

#define subqx_rr(XG, XS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x10))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x18))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x18))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x20))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x20))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x28))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x28))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x30))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x30))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x38))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x38))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define subqx_ld(XG, MS, DS)                                                \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(MS), W(DS))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x10))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x10))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x18))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x18))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x20))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x20))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x28))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x28))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x30))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x30))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x38))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x38))                              \
        stack_ld(Reax)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlqx_ri(XG, IS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlzx_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x10), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x18), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x20), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x28), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x30), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x38), W(IS))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MS), W(DS))                                       \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x20))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x28))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x30))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(MS), W(DS))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrqx_ri(XG, IS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzx_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x10), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x18), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x20), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x28), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x30), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x38), W(IS))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MS), W(DS))                                       \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x20))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x28))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x30))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(MS), W(DS))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))


#define shrqn_ri(XG, IS)                                                    \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x10), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x18), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x20), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x28), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x30), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x38), W(IS))                             \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MS), W(DS))                                       \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x20))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x28))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x30))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movqx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movqx_ld(W(XG), W(MS), W(DS))                                       \
        movqx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x38))                                    \
        stack_ld(Recx)                                                      \
        movqx_ld(W(XG), Mebp, inf_SCR01(0))

#else /* RT_512 >= 2 */

/* add (G = G + S) */

#define addqx_rr(XG, XS)                                                    \
        EMITW(0x100000C0 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x100000C0 | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XS), REG(XS)))                  \
        EMITW(0x100000C0 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XS), RYG(XS)))                  \
        EMITW(0x100000C0 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define addqx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

/* sub (G = G - S) */

#define subqx_rr(XG, XS)                                                    \
        EMITW(0x100004C0 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x100004C0 | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XS), REG(XS)))                  \
        EMITW(0x100004C0 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XS), RYG(XS)))                  \
        EMITW(0x100004C0 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define subqx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlqx_ri(XG, IS)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        shlqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shlqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000299 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0x100005C4 | MXM(RYG(XG), RYG(XG), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0x100005C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0x100005C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define svlqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x100005C4 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x100005C4 | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XS), REG(XS)))                  \
        EMITW(0x100005C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XS), RYG(XS)))                  \
        EMITW(0x100005C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define svlqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrqx_ri(XG, IS)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        shrqx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000299 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0x100006C4 | MXM(RYG(XG), RYG(XG), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0x100006C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0x100006C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define svrqx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x100006C4 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x100006C4 | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XS), REG(XS)))                  \
        EMITW(0x100006C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XS), RYG(XS)))                  \
        EMITW(0x100006C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define svrqx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))


#define shrqn_ri(XG, IS)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        shrqn_ld(W(XG), Mebp, inf_SCR01(0))

#define shrqn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000299 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0x100003C4 | MXM(RYG(XG), RYG(XG), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0x100003C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0x100003C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define svrqn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x100003C4 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x100003C4 | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XS), REG(XS)))                  \
        EMITW(0x100003C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XS), RYG(XS)))                  \
        EMITW(0x100003C4 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#define svrqn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(RYG(XG), RYG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XG), REG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(REG(XG), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XG), RYG(XG)))                  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(TmmQ,    TmmQ,    TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000496 | MXM(RYG(XG), TmmQ,    TmmQ))

#endif /* RT_512 >= 2 */

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE64_512  MN64_512   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_512  MF64_512   /*  all satisfy the condition */

/* #define S0(mask)    S1(mask)            (defined in 32_128-bit header) */
/* #define S1(mask)    S##mask             (defined in 32_128-bit header) */

#define SMN64_512(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000497 | MXM(TmmM, xs,  xs+16))                           \
        EMITW(0xF0000491 | MXM(TmmQ, xs,  xs+16))                           \
        EMITW(0xF0000497 | MXM(TmmM, TmmM, TmmQ))                           \
        EMITW(0x1000038C | MXM(TmmQ, 0x1F, 0x00))                           \
        EMITW(0x10000486 | MXM(TmmM, TmmM, TmmQ))                           \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF64_512(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000417 | MXM(TmmM, xs,  xs+16))                           \
        EMITW(0xF0000411 | MXM(TmmQ, xs,  xs+16))                           \
        EMITW(0xF0000417 | MXM(TmmM, TmmM, TmmQ))                           \
        EMITW(0x1000038C | MXM(TmmQ, 0x1F, 0x00))                           \
        EMITW(0x10000486 | MXM(TmmM, TmmM, TmmQ))                           \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define mkjqx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##64_512), EMPTY2)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndqs_rr(XD, XS)                                                    \
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003AF | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00003AC | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00003AC | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rndqs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AE | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AE | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

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
        EMITW(0xF00007E3 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00007E3 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00007E0 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00007E0 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvtqn_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00007E3 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VYL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VYL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00007E3 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VXL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VXL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00007E2 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */\
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VZL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VZL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00007E2 | MXM(RYG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrqs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndqs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvrqs_rr(XD, XS, mode)                                              \
        rnrqs_rr(W(XD), W(XS), mode)                                        \
        cvzqs_rr(W(XD), W(XD))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_512 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P64_512V2_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P64_128V4_H
#define RT_RTARCH_P64_128V4_H

#include "rtarch_p32_128v4.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p64_128v4.h: Implementation of Power fp64 VSX1/2 instructions.
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

#if defined (RT_128) && (RT_128 >= 2)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   VSX   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov (D = S) */

#define movjx_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))

#define movjx_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movjx_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000799 | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvjx_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))

#define mmvjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define mmvjx_st(XS, MG, DG)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000799 | MXM(TmmM,    Teax & (MOD(MG) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* and (G = G & S) */

#define andjx_rr(XG, XS)                                                    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), REG(XS)))

#define andjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000417 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* ann (G = ~G & S) */

#define annjx_rr(XG, XS)                                                    \
        EMITW(0xF0000457 | MXM(REG(XG), REG(XS), REG(XG)))

#define annjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000457 | MXM(REG(XG), TmmM,    REG(XG)))/* ^ == -1 if true */

/* orr (G = G | S) */

#define orrjx_rr(XG, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), REG(XS)))

#define orrjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000497 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* orn (G = ~G | S) */

#if (RT_128 < 4)

#define ornjx_rr(XG, XS)                                                    \
        notjx_rx(W(XG))                                                     \
        orrjx_rr(W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        notjx_rx(W(XG))                                                     \
        orrjx_ld(W(XG), W(MS), W(DS))

#else /* RT_128 >= 4 */

#define ornjx_rr(XG, XS)                                                    \
        EMITW(0xF0000557 | MXM(REG(XG), REG(XS), REG(XG)))

#define ornjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000557 | MXM(REG(XG), TmmM,    REG(XG)))/* ^ == -1 if true */

#endif /* RT_128 >= 4 */

/* xor (G = G ^ S) */

#define xorjx_rr(XG, XS)                                                    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), REG(XS)))

#define xorjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00004D7 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* not (G = ~G) */

#define notjx_rx(XG)                                                        \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

/**************   packed double precision floating point (SIMD)   *************/

/* neg (G = -G) */

#define negjs_rx(XG)                                                        \
        EMITW(0xF00007E7 | MXM(REG(XG), 0x00,    REG(XG)))

/* add (G = G + S) */

#define addjs_rr(XG, XS)                                                    \
        EMITW(0xF0000307 | MXM(REG(XG), REG(XG), REG(XS)))

#define addjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000307 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subjs_rr(XG, XS)                                                    \
        EMITW(0xF0000347 | MXM(REG(XG), REG(XG), REG(XS)))

#define subjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000347 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* mul (G = G * S) */

#define muljs_rr(XG, XS)                                                    \
        EMITW(0xF0000387 | MXM(REG(XG), REG(XG), REG(XS)))

#define muljs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000387 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* div (G = G / S) */

#define divjs_rr(XG, XS)                                                    \
        EMITW(0xF00003C7 | MXM(REG(XG), REG(XG), REG(XS)))

#define divjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003C7 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* sqr (D = sqrt S) */

#define sqrjs_rr(XD, XS)                                                    \
        EMITW(0xF000032F | MXM(REG(XD), 0x00,    REG(XS)))

#define sqrjs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000032F | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcejs_rr(XD, XS)                                                    \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsejs_rr(XD, XS)                                                    \
        sqrjs_rr(W(XD), W(XS))                                              \
        movjx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movjx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divjs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmajs_rr(XG, XS, XT)                                                \
        EMITW(0xF000030F | MXM(REG(XG), REG(XS), REG(XT)))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000030F | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsjs_rr(XG, XS, XT)                                                \
        EMITW(0xF000078F | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000078F | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minjs_rr(XG, XS)                                                    \
        EMITW(0xF0000747 | MXM(REG(XG), REG(XG), REG(XS)))

#define minjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000747 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* max (G = G > S ? G : S) */

#define maxjs_rr(XG, XS)                                                    \
        EMITW(0xF0000707 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000707 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* cmp (G = G ? S) */

#define ceqjs_rr(XG, XS)                                                    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define cnejs_rr(XG, XS)                                                    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cnejs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cltjs_rr(XG, XS)                                                    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XS), REG(XG)))

#define cltjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XG), TmmM,    REG(XG)))/* ^ == -1 if true */

#define clejs_rr(XG, XS)                                                    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XS), REG(XG)))

#define clejs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XG), TmmM,    REG(XG)))/* ^ == -1 if true */

#define cgtjs_rr(XG, XS)                                                    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtjs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define cgejs_rr(XG, XS)                                                    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgejs_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000367 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000367 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvzjs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000763 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000763 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpjs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00003A7 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003A7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvpjs_rr(XD, XS)     /* round towards +inf */                       \
        rnpjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmjs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF00003E7 | MXM(REG(XD), 0x00,    REG(XS)))

#define rnmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003E7 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvmjs_rr(XD, XS)     /* round towards -inf */                       \
        rnmjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    REG(XS)))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvnjs_rr(XD, XS)     /* round towards near */                       \
        rnnjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvnjs_ld(XD, MS, DS) /* round towards near */                       \
        rnnjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnjn_rr(XD, XS)     /* round towards near */                       \
        cvtjn_rr(W(XD), W(XS))

#define cvnjn_ld(XD, MS, DS) /* round towards near */                       \
        cvtjn_ld(W(XD), W(MS), W(DS))

/**************************   packed integer (SIMD)   *************************/

#if (RT_128 < 4)

/* add (G = G + S) */

#define addjx_rr(XG, XS)                                                    \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define addjx_ld(XG, MS, DS)                                                \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

/* sub (G = G - S) */

#define subjx_rr(XG, XS)                                                    \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define subjx_ld(XG, MS, DS)                                                \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movzx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subzx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        stack_ld(Reax)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shljx_ri(XG, IS)                                                    \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlzx_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shlzx_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MS), W(DS))                                       \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrjx_ri(XG, IS)                                                    \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzx_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shrzx_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MS), W(DS))                                       \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzx_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))


#define shrjn_ri(XG, IS)                                                    \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrzn_mi(Mebp,  inf_SCR01(0x00), W(IS))                             \
        shrzn_mi(Mebp,  inf_SCR01(0x08), W(IS))                             \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  W(MS), W(DS))                                       \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movjx_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movjx_ld(W(XG), W(MS), W(DS))                                       \
        movjx_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movzx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrzn_mx(Mebp,  inf_SCR01(0x08))                                    \
        stack_ld(Recx)                                                      \
        movjx_ld(W(XG), Mebp, inf_SCR01(0))

#else /* RT_128 >= 4 */

/* add (G = G + S) */

#define addjx_rr(XG, XS)                                                    \
        EMITW(0x100000C0 | MXM(REG(XG), REG(XG), REG(XS)))

#define addjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100000C0 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subjx_rr(XG, XS)                                                    \
        EMITW(0x100004C0 | MXM(REG(XG), REG(XG), REG(XS)))

#define subjx_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100004C0 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shljx_ri(XG, IS)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        shljx_ld(W(XG), Mebp, inf_SCR01(0))

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000299 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x100005C4 | MXM(REG(XG), REG(XG), REG(XS)))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100005C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrjx_ri(XG, IS)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        shrjx_ld(W(XG), Mebp, inf_SCR01(0))

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000299 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x100006C4 | MXM(REG(XG), REG(XG), REG(XS)))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100006C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */


#define shrjn_ri(XG, IS)                                                    \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        shrjn_ld(W(XG), Mebp, inf_SCR01(0))

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000299 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x100003C4 | MXM(REG(XG), REG(XG), REG(XS)))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0x100003C4 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#endif /* RT_128 >= 4 */

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE64_128  MN64_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL64_128  MF64_128   /*  all satisfy the condition */

/* #define S0(mask)    S1(mask)            (defined in 32_128-bit header) */
/* #define S1(mask)    S##mask             (defined in 32_128-bit header) */

#define SMN64_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF64_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define mkjjx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x10000486 | MXM(REG(XS), REG(XS), TmmQ))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##64_128), EMPTY2)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndjs_rr(XD, XS)                                                    \
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    REG(XS)))

#define rndjs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00003AF | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

#define cvtjs_rr(XD, XS)                                                    \
        rndjs_rr(W(XD), W(XS))                                              \
        cvzjs_rr(W(XD), W(XD))

#define cvtjs_ld(XD, MS, DS)                                                \
        rndjs_ld(W(XD), W(MS), W(DS))                                       \
        cvzjs_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtjn_rr(XD, XS)                                                    \
        EMITW(0xF00007E3 | MXM(REG(XD), 0x00,    REG(XS)))

#define cvtjn_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000699 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00007E3 | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnrjs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvrjs_rr(XD, XS, mode)                                              \
        rnrjs_rr(W(XD), W(XS), mode)                                        \
        cvzjs_rr(W(XD), W(XD))

/**************   scalar double precision floating point (SIMD)   *************/

#if (RT_128 < 4)

/* mov (D = S) */

#define movtx_rr(XD, XS)                                                    \
        EMITW(0xFC000090 | MXM(REG(XD), 0x00,    REG(XS)))

#define movtx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(REG(XD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movtx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xD8000000 | MDM(REG(XS), MOD(MD), VAL(DD), B1(DD), P1(DD)))

/* add (G = G + S) */

#define addts_rr(XG, XS)                                                    \
        EMITW(0xFC00002A | MXM(REG(XG), REG(XG), REG(XS)))

#define addts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xFC00002A | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subts_rr(XG, XS)                                                    \
        EMITW(0xFC000028 | MXM(REG(XG), REG(XG), REG(XS)))

#define subts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xFC000028 | MXM(REG(XG), REG(XG), TmmM))

/* mul (G = G * S) */

#define mults_rr(XG, XS)                                                    \
        EMITW(0xFC000032 | MXM(REG(XG), REG(XG), 0x00) | REG(XS) << 6)

#define mults_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xFC000032 | MXM(REG(XG), REG(XG), 0x00) | TmmM << 6)

/* div (G = G / S) */

#define divts_rr(XG, XS)                                                    \
        EMITW(0xFC000024 | MXM(REG(XG), REG(XG), REG(XS)))

#define divts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xFC000024 | MXM(REG(XG), REG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqrts_rr(XD, XS)                                                    \
        EMITW(0xFC00002C | MXM(REG(XD), 0x00,    REG(XS)))

#define sqrts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xFC00002C | MXM(REG(XD), 0x00,    TmmM))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcets_rr(XD, XS)                                                    \
        movtx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsets_rr(XD, XS)                                                    \
        sqrts_rr(W(XD), W(XS))                                              \
        movtx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR02(0))

#define rssts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMITW(0xFC00003A | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0xFC00003A | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMITW(0xFC00003C | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0xFC00003C | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define mints_rr(XG, XS)                                                    \
        EMITW(0xF0000740 | MXM(REG(XG), REG(XG), REG(XS)))

#define mints_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000740 | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxts_rr(XG, XS)                                                    \
        EMITW(0xF0000700 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000700 | MXM(REG(XG), REG(XG), TmmM))

/* cmp (G = G ? S) */

#define ceqts_rr(XG, XS)                                                    \
        EMITW(0xF0000318 | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000318 | MXM(REG(XG), REG(XG), TmmM))

#define cnets_rr(XG, XS)                                                    \
        EMITW(0xF0000318 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000510 | MXM(REG(XG), REG(XG), REG(XG)))

#define cnets_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000318 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0xF0000510 | MXM(REG(XG), REG(XG), REG(XG)))

#define cltts_rr(XG, XS)                                                    \
        EMITW(0xF0000358 | MXM(REG(XG), REG(XS), REG(XG)))

#define cltts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000358 | MXM(REG(XG), TmmM,    REG(XG)))

#define clets_rr(XG, XS)                                                    \
        EMITW(0xF0000398 | MXM(REG(XG), REG(XS), REG(XG)))

#define clets_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000398 | MXM(REG(XG), TmmM,    REG(XG)))

#define cgtts_rr(XG, XS)                                                    \
        EMITW(0xF0000358 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000358 | MXM(REG(XG), REG(XG), TmmM))

#define cgets_rr(XG, XS)                                                    \
        EMITW(0xF0000398 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgets_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xC8000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xF0000398 | MXM(REG(XG), REG(XG), TmmM))

#else /* RT_128 >= 4 */

/* mov (D = S) */

#define movtx_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))

#define movtx_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(REG(XD), Teax & (MOD(MS) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

#define movtx_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000599 | MXM(REG(XS), Teax & (MOD(MD) == TPxx), TPxx))    \
                                                       /* ^ == -1 if true */

/* add (G = G + S) */

#define addts_rr(XG, XS)                                                    \
        EMITW(0xF0000107 | MXM(REG(XG), REG(XG), REG(XS)))

#define addts_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000107 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* sub (G = G - S) */

#define subts_rr(XG, XS)                                                    \
        EMITW(0xF0000147 | MXM(REG(XG), REG(XG), REG(XS)))

#define subts_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000147 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* mul (G = G * S) */

#define mults_rr(XG, XS)                                                    \
        EMITW(0xF0000187 | MXM(REG(XG), REG(XG), REG(XS)))

#define mults_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000187 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* div (G = G / S) */

#define divts_rr(XG, XS)                                                    \
        EMITW(0xF00001C7 | MXM(REG(XG), REG(XG), REG(XS)))

#define divts_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF00001C7 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* sqr (D = sqrt S) */

#define sqrts_rr(XD, XS)                                                    \
        EMITW(0xF000012F | MXM(REG(XD), 0x00,    REG(XS)))

#define sqrts_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000012F | MXM(REG(XD), 0x00,    TmmM))/* ^ == -1 if true */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcets_rr(XD, XS)                                                    \
        movtx_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsets_rr(XD, XS)                                                    \
        sqrts_rr(W(XD), W(XS))                                              \
        movtx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movtx_ld(W(XD), Mebp, inf_GPC01_64)                                 \
        divts_ld(W(XD), Mebp, inf_SCR02(0))

#define rssts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMITW(0xF000010F | MXM(REG(XG), REG(XS), REG(XT)))

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000010F | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMITW(0xF000058F | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MT) == TPxx), TPxx))    \
        EMITW(0xF000058F | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define mints_rr(XG, XS)                                                    \
        EMITW(0xF0000547 | MXM(REG(XG), REG(XG), REG(XS)))

#define mints_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000547 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* max (G = G > S ? G : S) */

#define maxts_rr(XG, XS)                                                    \
        EMITW(0xF0000507 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxts_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF0000507 | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

/* cmp (G = G ? S) */

#define ceqts_rr(XG, XS)                                                    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqts_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define cnets_rr(XG, XS)                                                    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cnets_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000031F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */\
        EMITW(0xF0000517 | MXM(REG(XG), REG(XG), REG(XG)))

#define cltts_rr(XG, XS)                                                    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XS), REG(XG)))

#define cltts_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XG), TmmM,    REG(XG)))/* ^ == -1 if true */

#define clets_rr(XG, XS)                                                    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XS), REG(XG)))

#define clets_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XG), TmmM,    REG(XG)))/* ^ == -1 if true */

#define cgtts_rr(XG, XS)                                                    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtts_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000035F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#define cgets_rr(XG, XS)                                                    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XG), REG(XS)))

#define cgets_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000499 | MXM(TmmM,    Teax & (MOD(MS) == TPxx), TPxx))    \
        EMITW(0xF000039F | MXM(REG(XG), REG(XG), TmmM))/* ^ == -1 if true */

#endif /* RT_128 >= 4 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P64_128V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

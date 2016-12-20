/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M64_128V1_H
#define RT_RTARCH_M64_128V1_H

#include "rtarch_m32_128v1.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m64_128v1.h: Implementation of MIPS fp64 MSA instructions.
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

#if defined (RT_128) && (RT_128 != 0)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   MSA   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov (D = S) */

#define movjx_rr(XD, XS)                                                    \
        EMITW(0x78BE0019 | MXM(REG(XD), REG(XS), 0x00))

#define movjx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movjx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x78000027 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvjx_rr(XG, XS)                                                    \
        EMITW(0x7880001E | MXM(REG(XG), REG(XS), Tmm0))

#define mmvjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7880001E | MXM(REG(XG), TmmM,    Tmm0))

#define mmvjx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7880001E | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x78000027 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* and (G = G & S) */

#define andjx_rr(XG, XS)                                                    \
        EMITW(0x7800001E | MXM(REG(XG), REG(XG), REG(XS)))

#define andjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7800001E | MXM(REG(XG), REG(XG), TmmM))

/* ann (G = ~G & S) */

#define annjx_rr(XG, XS)                                                    \
        EMITW(0x78C0001E | MXM(REG(XG), REG(XS), TmmZ))

#define annjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001E | MXM(REG(XG), TmmM,    TmmZ))

/* orr (G = G | S) */

#define orrjx_rr(XG, XS)                                                    \
        EMITW(0x7820001E | MXM(REG(XG), REG(XG), REG(XS)))

#define orrjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7820001E | MXM(REG(XG), REG(XG), TmmM))

/* orn (G = ~G | S) */

#define ornjx_rr(XG, XS)                                                    \
        notjx_rx(W(XG))                                                     \
        orrjx_rr(W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        notjx_rx(W(XG))                                                     \
        orrjx_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorjx_rr(XG, XS)                                                    \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), REG(XS)))

#define xorjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), TmmM))

/* not (G = ~G) */

#define notjx_rx(XG)                                                        \
        EMITW(0x7840001E | MXM(REG(XG), TmmZ,    REG(XG)))

/**************   packed double precision floating point (SIMD)   *************/

/* neg (G = -G) */

#define negjs_rx(XG)                                                        \
        movjx_xm(Mebp, inf_GPC06_64)                                        \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), TmmM))

#define movjx_xm(MS, DS) /* not portable, do not use outside */             \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))

/* add (G = G + S) */

#define addjs_rr(XG, XS)                                                    \
        EMITW(0x7820001B | MXM(REG(XG), REG(XG), REG(XS)))

#define addjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7820001B | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subjs_rr(XG, XS)                                                    \
        EMITW(0x7860001B | MXM(REG(XG), REG(XG), REG(XS)))

#define subjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7860001B | MXM(REG(XG), REG(XG), TmmM))

/* mul (G = G * S) */

#define muljs_rr(XG, XS)                                                    \
        EMITW(0x78A0001B | MXM(REG(XG), REG(XG), REG(XS)))

#define muljs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78A0001B | MXM(REG(XG), REG(XG), TmmM))

/* div (G = G / S) */

#define divjs_rr(XG, XS)                                                    \
        EMITW(0x78E0001B | MXM(REG(XG), REG(XG), REG(XS)))

#define divjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78E0001B | MXM(REG(XG), REG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqrjs_rr(XD, XS)                                                    \
        EMITW(0x7B27001E | MXM(REG(XD), REG(XS), 0x00))

#define sqrjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B27001E | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcejs_rr(XD, XS)                                                    \
        EMITW(0x7B2B001E | MXM(REG(XD), REG(XS), 0x00))

#define rcsjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsejs_rr(XD, XS)                                                    \
        EMITW(0x7B29001E | MXM(REG(XD), REG(XS), 0x00))

#define rssjs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmajs_rr(XG, XS, XT)                                                \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmajs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsjs_rr(XG, XS, XT)                                                \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsjs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minjs_rr(XG, XS)                                                    \
        EMITW(0x7B20001B | MXM(REG(XG), REG(XG), REG(XS)))

#define minjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B20001B | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxjs_rr(XG, XS)                                                    \
        EMITW(0x7BA0001B | MXM(REG(XG), REG(XG), REG(XS)))

#define maxjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7BA0001B | MXM(REG(XG), REG(XG), TmmM))

/* cmp (G = G ? S) */

#define ceqjs_rr(XG, XS)                                                    \
        EMITW(0x78A0001A | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78A0001A | MXM(REG(XG), REG(XG), TmmM))

#define cnejs_rr(XG, XS)                                                    \
        EMITW(0x78E0001C | MXM(REG(XG), REG(XG), REG(XS)))

#define cnejs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78E0001C | MXM(REG(XG), REG(XG), TmmM))

#define cltjs_rr(XG, XS)                                                    \
        EMITW(0x7920001A | MXM(REG(XG), REG(XG), REG(XS)))

#define cltjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7920001A | MXM(REG(XG), REG(XG), TmmM))

#define clejs_rr(XG, XS)                                                    \
        EMITW(0x79A0001A | MXM(REG(XG), REG(XG), REG(XS)))

#define clejs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x79A0001A | MXM(REG(XG), REG(XG), TmmM))

#define cgtjs_rr(XG, XS)                                                    \
        EMITW(0x7920001A | MXM(REG(XG), REG(XS), REG(XG)))

#define cgtjs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7920001A | MXM(REG(XG), TmmM,    REG(XG)))

#define cgejs_rr(XG, XS)                                                    \
        EMITW(0x79A0001A | MXM(REG(XG), REG(XS), REG(XG)))

#define cgejs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x79A0001A | MXM(REG(XG), TmmM,    REG(XG)))

/**************************   packed integer (SIMD)   *************************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnzjs_rr(XD, XS)     /* round towards zero */                       \
        cvzjs_rr(W(XD), W(XS))                                              \
        cvnjn_rr(W(XD), W(XD))

#define rnzjs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzjs_ld(W(XD), W(MS), W(DS))                                       \
        cvnjn_rr(W(XD), W(XD))

#define cvzjs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x7B23001E | MXM(REG(XD), REG(XS), 0x00))

#define cvzjs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B23001E | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnpjs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rnpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cvpjs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpjs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnmjs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rnmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cvmjs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmjs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtjs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rnnjs_rr(XD, XS)     /* round towards near */                       \
        rndjs_rr(W(XD), W(XS))

#define rnnjs_ld(XD, MS, DS) /* round towards near */                       \
        rndjs_ld(W(XD), W(MS), W(DS))

#define cvnjs_rr(XD, XS)     /* round towards near */                       \
        cvtjs_rr(W(XD), W(XS))

#define cvnjs_ld(XD, MS, DS) /* round towards near */                       \
        cvtjs_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnjn_rr(XD, XS)     /* round towards near */                       \
        cvtjn_rr(W(XD), W(XS))

#define cvnjn_ld(XD, MS, DS) /* round towards near */                       \
        cvtjn_ld(W(XD), W(MS), W(DS))

/* add (G = G + S) */

#define addjx_rr(XG, XS)                                                    \
        EMITW(0x7860000E | MXM(REG(XG), REG(XG), REG(XS)))

#define addjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7860000E | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subjx_rr(XG, XS)                                                    \
        EMITW(0x78E0000E | MXM(REG(XG), REG(XG), REG(XS)))

#define subjx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78E0000E | MXM(REG(XG), REG(XG), TmmM))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shljx_ri(XG, IS)                                                    \
        EMITW(0x78000009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)

#define shljx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B03001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7860000D | MXM(REG(XG), REG(XG), TmmM))

#define svljx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x7860000D | MXM(REG(XG), REG(XG), REG(XS)))

#define svljx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7860000D | MXM(REG(XG), REG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrjx_ri(XG, IS)                                                    \
        EMITW(0x79000009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)

#define shrjx_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B03001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7960000D | MXM(REG(XG), REG(XG), TmmM))

#define svrjx_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x7960000D | MXM(REG(XG), REG(XG), REG(XS)))

#define svrjx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7960000D | MXM(REG(XG), REG(XG), TmmM))


#define shrjn_ri(XG, IS)                                                    \
        EMITW(0x78800009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x3F & VAL(IS)) << 16)

#define shrjn_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xDC000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B03001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x78E0000D | MXM(REG(XG), REG(XG), TmmM))

#define svrjn_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x78E0000D | MXM(REG(XG), REG(XG), REG(XS)))

#define svrjn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78E0000D | MXM(REG(XG), REG(XG), TmmM))

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
        ASM_BEG ASM_OP2( bz.v, xs, lb) ASM_END

#define SMF64_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(bnz.w, xs, lb) ASM_END

#define mkjjx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, MOD(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##64_128), EMPTY2)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 64-bit signed int range */

#define rndjs_rr(XD, XS)                                                    \
        EMITW(0x7B2D001E | MXM(REG(XD), REG(XS), 0x00))

#define rndjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B2D001E | MXM(REG(XD), TmmM,    0x00))

#define cvtjs_rr(XD, XS)                                                    \
        EMITW(0x7B39001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtjs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B39001E | MXM(REG(XD), TmmM,    0x00))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtjn_rr(XD, XS)                                                    \
        EMITW(0x7B3D001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtjn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B3D001E | MXM(REG(XD), TmmM,    0x00))

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
        FCTRL_ENTER(mode)                                                   \
        cvtjs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

/**************   scalar double precision floating point (SIMD)   *************/

/* mov (D = S) */

#define movtx_rr(XD, XS)                                                    \
        EMITW(0x46200006 | MXM(REG(XD), REG(XS), 0x00))

#define movtx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(REG(XD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movtx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xF4000000 | MDM(REG(XS), MOD(MD), VAL(DD), B1(DD), P1(DD)))

/* add (G = G + S) */

#define addts_rr(XG, XS)                                                    \
        EMITW(0x46200000 | MXM(REG(XG), REG(XG), REG(XS)))

#define addts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x46200000 | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subts_rr(XG, XS)                                                    \
        EMITW(0x46200001 | MXM(REG(XG), REG(XG), REG(XS)))

#define subts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x46200001 | MXM(REG(XG), REG(XG), TmmM))

/* mul (G = G * S) */

#define mults_rr(XG, XS)                                                    \
        EMITW(0x46200002 | MXM(REG(XG), REG(XG), REG(XS)))

#define mults_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x46200002 | MXM(REG(XG), REG(XG), TmmM))

/* div (G = G / S) */

#define divts_rr(XG, XS)                                                    \
        EMITW(0x46200003 | MXM(REG(XG), REG(XG), REG(XS)))

#define divts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x46200003 | MXM(REG(XG), REG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqrts_rr(XD, XS)                                                    \
        EMITW(0x46200004 | MXM(REG(XD), REG(XS), 0x00))

#define sqrts_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x46200004 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcets_rr(XD, XS)                                                    \
        EMITW(0x46200015 | MXM(REG(XD), REG(XS), 0x00))

#define rcsts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsets_rr(XD, XS)                                                    \
        EMITW(0x46200016 | MXM(REG(XD), REG(XS), 0x00))

#define rssts_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* pre-r6 */
#if (defined (RT_M32) && RT_M32 < 6) || (defined (RT_M64) && RT_M64 < 6)

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7920001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x7960001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define mints_rr(XG, XS)                                                    \
        EMITW(0x7B20001B | MXM(REG(XG), REG(XG), REG(XS)))

#define mints_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B20001B | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxts_rr(XG, XS)                                                    \
        EMITW(0x7BA0001B | MXM(REG(XG), REG(XG), REG(XS)))

#define maxts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7BA0001B | MXM(REG(XG), REG(XG), TmmM))

#else  /* r6 */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmats_rr(XG, XS, XT)                                                \
        EMITW(0x46200018 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmats_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200018 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsts_rr(XG, XS, XT)                                                \
        EMITW(0x46200019 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsts_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0x46200019 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define mints_rr(XG, XS)                                                    \
        EMITW(0x4620001C | MXM(REG(XG), REG(XG), REG(XS)))

#define mints_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4620001C | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxts_rr(XG, XS)                                                    \
        EMITW(0x4620001E | MXM(REG(XG), REG(XG), REG(XS)))

#define maxts_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xD4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4620001E | MXM(REG(XG), REG(XG), TmmM))

#endif /* r6 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M64_128V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

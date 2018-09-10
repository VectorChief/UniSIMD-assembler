/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_SVEX1V1_H
#define RT_RTARCH_A32_SVEX1V1_H

#include "rtarch_a64.h"

#define RT_SIMD_REGS        32
#define RT_SIMD_ALIGN       64
#define RT_SIMD_WIDTH64     8
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v
#define RT_SIMD_WIDTH32     16
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a32_SVEx1v1.h: Implementation of ARMv8:AArch64 fp32 SVE instructions.
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

#ifndef RT_RTARCH_A64_128X2V1_H
#undef  RT_128X2
#define RT_128X2  1
#include "rtarch_a64_128x2v1.h"
#endif /* RT_RTARCH_A64_128X2V1_H */

#undef  sregs_sa
#undef  sregs_la

/* selectors  */

#define  F1(val, tp1, tp2)  F1##tp1

/* displacement encoding SIMD(TP1) */

#define F10(dp) (((dp)/(RT_SIMD/8)&0x07) << 10 | ((dp)/(RT_SIMD/8)&0xF8) << 13)

#define F11(dp) (0x00000000)

#define F12(dp) (0x00000000)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SVE   ***********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmox_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movrs_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), REG(XS)))

#define movox_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(REG(XD), MOD(MS), VAL(DS), B1(DS), F1(DS)))

#define movox_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xE5804000 | MPM(REG(XS), MOD(MD), VAL(DD), B1(DD), F1(DD)))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked elems */

#define mmvox_rr(XG, XS)                                                    \
        EMITW(0x2480A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x05A0C400 | MXM(REG(XG), REG(XS), REG(XG)))

#define mmvox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x2480A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x05A0C400 | MXM(REG(XG), TmmM,    REG(XG)))

#define mmvox_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x2480A000 | MXM(0x01,    Tmm0,    TmmQ))                     \
        EMITW(0x05A0C400 | MXM(TmmM,    REG(XS), TmmM))                     \
        EMITW(0xE5804000 | MPM(TmmM,    MOD(MG), VAL(DG), B1(DG), F1(DG)))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EMITW(0x04203000 | MXM(REG(XD), REG(XS), REG(XT)))

#define andox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04203000 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EMITW(0x04E03000 | MXM(REG(XD), REG(XT), REG(XS)))

#define annox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04E03000 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04603000 | MXM(REG(XD), REG(XS), TmmM))

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
        EMITW(0x04A03000 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notox_rx(XG)                                                        \
        notox_rr(W(XG), W(XG))

#define notox_rr(XD, XS)                                                    \
        EMITW(0x049EA000 | MXM(REG(XD), REG(XS), 0x00))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negos_rx(XG)                                                        \
        negos_rr(W(XG), W(XG))

#define negos_rr(XD, XS)                                                    \
        EMITW(0x049DA000 | MXM(REG(XD), REG(XS), 0x00))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addos_rr(XG, XS)                                                    \
        addos3rr(W(XG), W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addos3ld(W(XG), W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        EMITW(0x65800000 | MXM(REG(XD), REG(XS), REG(XT)))

#define addos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65800000 | MXM(REG(XD), REG(XS), TmmM))

        /* adp, adh defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subos_rr(XG, XS)                                                    \
        subos3rr(W(XG), W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subos3ld(W(XG), W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        EMITW(0x65800400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65800400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulos_rr(XG, XS)                                                    \
        mulos3rr(W(XG), W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulos3ld(W(XG), W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        EMITW(0x65800800 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65800800 | MXM(REG(XD), REG(XS), TmmM))

        /* mlp, mlh defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divos_rr(XG, XS)                                                    \
        EMITW(0x658D8000 | MXM(REG(XG), REG(XS), 0x00))

#define divos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x658D8000 | MXM(REG(XG), TmmM,    0x00))

#define divos3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        divos_rr(W(XD), W(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        divos_ld(W(XD), W(MT), W(DT))

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0x658DA000 | MXM(REG(XD), REG(XS), 0x00))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x658DA000 | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        EMITW(0x658E3000 | MXM(REG(XD), REG(XS), 0x00))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x65801800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65800800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        EMITW(0x658F3000 | MXM(REG(XD), REG(XS), 0x00))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x65800800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65801C00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x65800800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0x65A00000 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65A00000 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0x65A02000 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65A02000 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minos_rr(XG, XS)                                                    \
        EMITW(0x65878000 | MXM(REG(XG), REG(XS), 0x00))

#define minos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65878000 | MXM(REG(XG), TmmM,    0x00))

#define minos3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        minos_rr(W(XD), W(XT))

#define minos3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        minos_ld(W(XD), W(MT), W(DT))

        /* mnp, mnh defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0x65868000 | MXM(REG(XG), REG(XS), 0x00))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x65868000 | MXM(REG(XG), TmmM,    0x00))

#define maxos3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        maxos_rr(W(XD), W(XT))

#define maxos3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        maxos_ld(W(XD), W(MT), W(DT))

        /* mxp, mxh defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqos_rr(XG, XS)                                                    \
        ceqos3rr(W(XG), W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqos3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        EMITW(0x65806000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define ceqos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65806000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneos_rr(XG, XS)                                                    \
        cneos3rr(W(XG), W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cneos3ld(W(XG), W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        EMITW(0x65806010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cneos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65806010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltos_rr(XG, XS)                                                    \
        cltos3rr(W(XG), W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltos3ld(W(XG), W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        EMITW(0x65804010 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cltos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65804010 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleos_rr(XG, XS)                                                    \
        cleos3rr(W(XG), W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        cleos3ld(W(XG), W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        EMITW(0x65804000 | MXM(0x01,    REG(XT), REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cleos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65804000 | MXM(0x01,    TmmM,    REG(XS)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtos_rr(XG, XS)                                                    \
        cgtos3rr(W(XG), W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        EMITW(0x65804010 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cgtos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65804010 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeos_rr(XG, XS)                                                    \
        cgeos3rr(W(XG), W(XG), W(XS))

#define cgeos_ld(XG, MS, DS)                                                \
        cgeos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeos3rr(XD, XS, XT)                                                \
        EMITW(0x65804000 | MXM(0x01,    REG(XS), REG(XT)))                  \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

#define cgeos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x65804000 | MXM(0x01,    REG(XS), TmmM))                     \
        EMITW(0x04A03000 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x05A0C400 | MXM(REG(XD), TmmQ,    REG(XD)))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_SVE     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_SVE     0x01    /*  all satisfy the condition */

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x04982000 | MXM(TmmM,    REG(XS), 0x00)|                     \
                       RT_SIMD_MASK_##mask##32_SVE<<17)                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask##32_SVE))                     \
        jezxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x6583A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x6583A000 | MXM(REG(XD), TmmM,    0x00))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x659CA000 | MXM(REG(XD), REG(XS), 0x00))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x659CA000 | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x6581A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x6581A000 | MXM(REG(XD), TmmM,    0x00))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x6582A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x6582A000 | MXM(REG(XD), TmmM,    0x00))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x6580A000 | MXM(REG(XD), REG(XS), 0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x6580A000 | MXM(REG(XD), TmmM,    0x00))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        rnnos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        rnnos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0x6587A000 | MXM(REG(XD), REG(XS), 0x00))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x6587A000 | MXM(REG(XD), TmmM,    0x00))

#define cvtos_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvtos_ld(XD, MS, DS)                                                \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        EMITW(0x6594A000 | MXM(REG(XD), REG(XS), 0x00))

#define cvton_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x6594A000 | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EMITW(0x6580A000 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                        RT_SIMD_MODE_##mode << 16)

#define cvros_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)                                        \
        cvzos_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addox_rr(XG, XS)                                                    \
        addox3rr(W(XG), W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addox3ld(W(XG), W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        EMITW(0x04A00000 | MXM(REG(XD), REG(XS), REG(XT)))

#define addox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04A00000 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subox_rr(XG, XS)                                                    \
        subox3rr(W(XG), W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subox3ld(W(XG), W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        EMITW(0x04A00400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x04A00400 | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlox3ri(W(XG), W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05A03800 | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x04938000 | MXM(REG(XG), TmmM,    0x00))

#define shlox3ri(XD, XS, IT)                                                \
        EMITW(0x04609C00 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shlox3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        shlox_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shrox3ri(W(XG), W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05A03800 | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x04918000 | MXM(REG(XG), TmmM,    0x00))

#define shrox3ri(XD, XS, IT)                                                \
        EMITW(0x04609400 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000800) | (M(VAL(IT) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IT)) << 16)

#define shrox3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        shrox_ld(W(XD), W(MT), W(DT))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shron_ri(XG, IS)     /* emits shift-left for zero-immediate args */ \
        shron3ri(W(XG), W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB9400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x05A03800 | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x04908000 | MXM(REG(XG), TmmM,    0x00))

#define shron3ri(XD, XS, IT)                                                \
        EMITW(0x04609000 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (M(VAL(IT) == 0) & 0x00000C00) | (M(VAL(IT) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IT)) << 16)

#define shron3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        shron_ld(W(XD), W(MT), W(DT))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04938000 | MXM(REG(XG), REG(XS), 0x00))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x04938000 | MXM(REG(XG), TmmM,    0x00))

#define svlox3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        svlox_rr(W(XD), W(XT))

#define svlox3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        svlox_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04918000 | MXM(REG(XG), REG(XS), 0x00))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x04918000 | MXM(REG(XG), TmmM,    0x00))

#define svrox3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        svrox_rr(W(XD), W(XT))

#define svrox3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        svrox_ld(W(XD), W(MT), W(DT))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x04908000 | MXM(REG(XG), REG(XS), 0x00))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x85804000 | MPM(TmmM,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x04908000 | MXM(REG(XG), TmmM,    0x00))

#define svron3rr(XD, XS, XT)                                                \
        movox_rr(W(XD), W(XS))                                              \
        svron_rr(W(XD), W(XT))

#define svron3ld(XD, XS, MT, DT)                                            \
        movox_rr(W(XD), W(XS))                                              \
        svron_ld(W(XD), W(MT), W(DT))

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
        movox_st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmG, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmH, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmI, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmJ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmK, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmL, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmM, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmN, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmO, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmP, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmQ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmR, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmS, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_st(XmmT, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xE5804000 | MXM(TmmQ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xE5804000 | MXM(TmmM,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xE5800000 | MXM(0x00,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0xE5800000 | MXM(0x01,    Teax,    0x00))

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
        movox_ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmF, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmG, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmH, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmI, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmJ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmK, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmL, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmM, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmN, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmO, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmP, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmQ, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmR, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmS, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        movox_ld(XmmT, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x85804000 | MXM(TmmQ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x85804000 | MXM(TmmM,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x85800000 | MXM(0x00,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x85800000 | MXM(0x01,    Teax,    0x00))

#endif /* RT_SVEX1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A32_SVEX1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

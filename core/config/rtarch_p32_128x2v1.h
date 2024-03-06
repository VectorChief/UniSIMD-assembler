/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_128X2V1_H
#define RT_RTARCH_P32_128X2V1_H

#include "rtarch_p64.h"

#define RT_SIMD_REGS_256        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_128x2v1.h: Implementation of POWER fp32 VSX1/2 instruction pairs.
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

#if (RT_128X2 == 1 || RT_128X2 == 16) && (RT_SIMD_COMPAT_XMM > 0)

#ifndef RT_RTARCH_P64_128X1V1_H
#undef  RT_128X1
#define RT_128X1  1
#include "rtarch_p64_128x1v1.h"
#endif /* RT_RTARCH_P64_128X1V1_H */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmcx_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        elmix_st(W(XS), W(MD), W(DD))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movcx_rr(XD, XS)                                                    \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(REG(XD), T0xx,    TPxx))                     \
        EMITW(0x7C000619 | MXM(RYG(XD), T1xx,    TPxx))

#define movcx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(REG(XS), T0xx,    TPxx))                     \
        EMITW(0x7C000719 | MXM(RYG(XS), T1xx,    TPxx))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvcx_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mmvcx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), TmmM))

#define mmvcx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000043F | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    T1xx,    TPxx))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andcx_rr(XG, XS)                                                    \
        andcx3rr(W(XG), W(XG), W(XS))

#define andcx_ld(XG, MS, DS)                                                \
        andcx3ld(W(XG), W(XG), W(MS), W(DS))

#define andcx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anncx_rr(XG, XS)                                                    \
        anncx3rr(W(XG), W(XG), W(XS))

#define anncx_ld(XG, MS, DS)                                                \
        anncx3ld(W(XG), W(XG), W(MS), W(DS))

#define anncx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000457 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000457 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define anncx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000457 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000457 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrcx_rr(XG, XS)                                                    \
        orrcx3rr(W(XG), W(XG), W(XS))

#define orrcx_ld(XG, MS, DS)                                                \
        orrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrcx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#if (RT_SIMD_COMPAT_PW8 == 0)

#define orncx_rr(XG, XS)                                                    \
        notcx_rx(W(XG))                                                     \
        orrcx_rr(W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        notcx_rx(W(XG))                                                     \
        orrcx_ld(W(XG), W(MS), W(DS))

#define orncx3rr(XD, XS, XT)                                                \
        notcx_rr(W(XD), W(XS))                                              \
        orrcx_rr(W(XD), W(XT))

#define orncx3ld(XD, XS, MT, DT)                                            \
        notcx_rr(W(XD), W(XS))                                              \
        orrcx_ld(W(XD), W(MT), W(DT))

#else /* RT_SIMD_COMPAT_PW8 == 1 */

#define orncx_rr(XG, XS)                                                    \
        orncx3rr(W(XG), W(XG), W(XS))

#define orncx_ld(XG, MS, DS)                                                \
        orncx3ld(W(XG), W(XG), W(MS), W(DS))

#define orncx3rr(XD, XS, XT)                                                \
        EMITW(0xF0000557 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000557 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define orncx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000557 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000557 | MXM(RYG(XD), TmmM,    RYG(XS)))

#endif /* RT_SIMD_COMPAT_PW8 == 1 */

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorcx_rr(XG, XS)                                                    \
        xorcx3rr(W(XG), W(XG), W(XS))

#define xorcx_ld(XG, MS, DS)                                                \
        xorcx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorcx3rr(XD, XS, XT)                                                \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xorcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notcx_rx(XG)                                                        \
        notcx_rr(W(XG), W(XG))

#define notcx_rr(XD, XS)                                                    \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XS), RYG(XS)))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negcs_rx(XG)                                                        \
        negcs_rr(W(XG), W(XG))

#define negcs_rr(XD, XS)                                                    \
        EMITW(0xF00006E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00006E7 | MXM(RYG(XD), 0x00,    RYG(XS)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcs_rr(XG, XS)                                                    \
        addcs3rr(W(XG), W(XG), W(XS))

#define addcs_ld(XG, MS, DS)                                                \
        addcs3ld(W(XG), W(XG), W(MS), W(DS))

#define addcs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000207 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000207 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000207 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000207 | MXM(RYG(XD), RYG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcs_rr(XG, XS)                                                    \
        subcs3rr(W(XG), W(XG), W(XS))

#define subcs_ld(XG, MS, DS)                                                \
        subcs3ld(W(XG), W(XG), W(MS), W(DS))

#define subcs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000247 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000247 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000247 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000247 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcs_rr(XG, XS)                                                    \
        mulcs3rr(W(XG), W(XG), W(XS))

#define mulcs_ld(XG, MS, DS)                                                \
        mulcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000287 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000287 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000287 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000287 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divcs_rr(XG, XS)                                                    \
        divcs3rr(W(XG), W(XG), W(XS))

#define divcs_ld(XG, MS, DS)                                                \
        divcs3ld(W(XG), W(XG), W(MS), W(DS))

#define divcs3rr(XD, XS, XT)                                                \
        EMITW(0xF00002C7 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00002C7 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002C7 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002C7 | MXM(RYG(XD), RYG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    RYG(XS)))

#define sqrcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    TmmM))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcecs_rr(XD, XS)                                                    \
        EMITW(0xF000026B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000026B | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rcscs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF00006CD | MXM(REG(XS), REG(XG), TmmQ))                     \
        EMITW(0xF000020F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00006CD | MXM(RYG(XS), RYG(XG), TmmQ))                     \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsecs_rr(XD, XS)                                                    \
        EMITW(0xF000022B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022B | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rsscs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF0000287 | MXM(TmmM,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000285 | MXM(TmmQ,    REG(XG), TmmM))                     \
        EMITW(0xF00006CD | MXM(TmmM,    REG(XS), TmmQ))                     \
        EMITW(0xF000068F | MXM(REG(XG), TmmM,    TmmQ))                     \
        EMITW(0xF0000287 | MXM(TmmM,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000285 | MXM(TmmQ,    RYG(XG), TmmM))                     \
        EMITW(0xF00006CD | MXM(TmmM,    RYG(XS), TmmQ))                     \
        EMITW(0xF000068F | MXM(RYG(XG), TmmM,    TmmQ))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mincs_rr(XG, XS)                                                    \
        mincs3rr(W(XG), W(XG), W(XS))

#define mincs_ld(XG, MS, DS)                                                \
        mincs3ld(W(XG), W(XG), W(MS), W(DS))

#define mincs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000647 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000647 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mincs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000647 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000647 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxcs_rr(XG, XS)                                                    \
        maxcs3rr(W(XG), W(XG), W(XS))

#define maxcs_ld(XG, MS, DS)                                                \
        maxcs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcs3rr(XD, XS, XT)                                                \
        EMITW(0xF0000607 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000607 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000607 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000607 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcs_rr(XG, XS)                                                    \
        ceqcs3rr(W(XG), W(XG), W(XS))

#define ceqcs_ld(XG, MS, DS)                                                \
        ceqcs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcs3rr(XD, XS, XT)                                                \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecs_rr(XG, XS)                                                    \
        cnecs3rr(W(XG), W(XG), W(XS))

#define cnecs_ld(XG, MS, DS)                                                \
        cnecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecs3rr(XD, XS, XT)                                                \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define cnecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XD), RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltcs_rr(XG, XS)                                                    \
        cltcs3rr(W(XG), W(XG), W(XS))

#define cltcs_ld(XG, MS, DS)                                                \
        cltcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcs3rr(XD, XS, XT)                                                \
        EMITW(0xF000025F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF000025F | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clecs_rr(XG, XS)                                                    \
        clecs3rr(W(XG), W(XG), W(XS))

#define clecs_ld(XG, MS, DS)                                                \
        clecs3ld(W(XG), W(XG), W(MS), W(DS))

#define clecs3rr(XD, XS, XT)                                                \
        EMITW(0xF000029F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF000029F | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define clecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtcs_rr(XG, XS)                                                    \
        cgtcs3rr(W(XG), W(XG), W(XS))

#define cgtcs_ld(XG, MS, DS)                                                \
        cgtcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcs3rr(XD, XS, XT)                                                \
        EMITW(0xF000025F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000025F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgecs_rr(XG, XS)                                                    \
        cgecs3rr(W(XG), W(XG), W(XS))

#define cgecs_ld(XG, MS, DS)                                                \
        cgecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecs3rr(XD, XS, XT)                                                \
        EMITW(0xF000029F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000029F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(RYG(XD), RYG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_256  MN32_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256  MF32_256   /*  all satisfy the condition */

/* #define S0(mask)    S1(mask)            (defined in 32_128-bit header) */
/* #define S1(mask)    S##mask             (defined in 32_128-bit header) */

#define SMN32_256(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000497 | MXM(TmmM,    xs,      xs+16))                    \
        EMITW(0x10000486 | MXM(TmmM,    TmmM,    TmmQ))                     \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF32_256(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000417 | MXM(TmmM,    xs,      xs+16))                    \
        EMITW(0x10000486 | MXM(TmmM,    TmmM,    TmmQ))                     \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x1000038C | MXM(TmmQ,    0x1F,    0x00))                     \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##32_256), EMPTY2)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    TmmM))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    TmmM))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    TmmM))

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        rnpcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    TmmM))

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        rnmcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        rnncs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        rnncs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rndcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))

#define cvtcs_rr(XD, XS)                                                    \
        rndcs_rr(W(XD), W(XS))                                              \
        cvzcs_rr(W(XD), W(XD))

#define cvtcs_ld(XD, MS, DS)                                                \
        rndcs_ld(W(XD), W(MS), W(DS))                                       \
        cvzcs_rr(W(XD), W(XD))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0x1000020A | MXM(RYG(XD), 0x00,    RYG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define cvrcs_rr(XD, XS, mode)                                              \
        rnrcs_rr(W(XD), W(XS), mode)                                        \
        cvzcs_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncn_rr(XD, XS)     /* round towards near */                       \
        cvtcn_rr(W(XD), W(XS))

#define cvncn_ld(XD, MS, DS) /* round towards near */                       \
        cvtcn_ld(W(XD), W(MS), W(DS))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcn_rr(XD, XS)                                                    \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvtcn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    TmmM))

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvncx_rr(XD, XS)     /* round towards near */                       \
        cvtcx_rr(W(XD), W(XS))

#define cvncx_ld(XD, MS, DS) /* round towards near */                       \
        cvtcx_ld(W(XD), W(MS), W(DS))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtcx_rr(XD, XS)                                                    \
        EMITW(0xF00002A3 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A3 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvtcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002A3 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002A3 | MXM(RYG(XD), 0x00,    TmmM))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define ruzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    TmmM))

#define cuzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000223 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000223 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cuzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000223 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000223 | MXM(RYG(XD), 0x00,    TmmM))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupcs_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    TmmM))

#define cupcs_rr(XD, XS)     /* round towards +inf */                       \
        rupcs_rr(W(XD), W(XS))                                              \
        cuzcs_rr(W(XD), W(XD))

#define cupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        rupcs_ld(W(XD), W(MS), W(DS))                                       \
        cuzcs_rr(W(XD), W(XD))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumcs_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    TmmM))

#define cumcs_rr(XD, XS)     /* round towards -inf */                       \
        rumcs_rr(W(XD), W(XS))                                              \
        cuzcs_rr(W(XD), W(XD))

#define cumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        rumcs_ld(W(XD), W(MS), W(DS))                                       \
        cuzcs_rr(W(XD), W(XD))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runcs_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define runcs_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))

#define cuncs_rr(XD, XS)     /* round towards near */                       \
        runcs_rr(W(XD), W(XS))                                              \
        cuzcs_rr(W(XD), W(XD))

#define cuncs_ld(XD, MS, DS) /* round towards near */                       \
        runcs_ld(W(XD), W(MS), W(DS))                                       \
        cuzcs_rr(W(XD), W(XD))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudcs_rr(XD, XS)                                                    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rudcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))

#define cutcs_rr(XD, XS)                                                    \
        rudcs_rr(W(XD), W(XS))                                              \
        cuzcs_rr(W(XD), W(XD))

#define cutcs_ld(XD, MS, DS)                                                \
        rudcs_ld(W(XD), W(MS), W(DS))                                       \
        cuzcs_rr(W(XD), W(XD))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurcs_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0x1000020A | MXM(RYG(XD), 0x00,    RYG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define curcs_rr(XD, XS, mode)                                              \
        rurcs_rr(W(XD), W(XS), mode)                                        \
        cuzcs_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcx_rr(XG, XS)                                                    \
        addcx3rr(W(XG), W(XG), W(XS))

#define addcx_ld(XG, MS, DS)                                                \
        addcx3ld(W(XG), W(XG), W(MS), W(DS))

#define addcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000080 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000080 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000080 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000080 | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcx_rr(XG, XS)                                                    \
        subcx3rr(W(XG), W(XG), W(XS))

#define subcx_ld(XG, MS, DS)                                                \
        subcx3ld(W(XG), W(XG), W(MS), W(DS))

#define subcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000480 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000480 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000480 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000480 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#if (RT_SIMD_COMPAT_PW8 == 0)

#define mulcx_rr(XG, XS)                                                    \
        mulcx3rr(W(XG), W(XG), W(XS))

#define mulcx_ld(XG, MS, DS)                                                \
        mulcx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcx3rr(XD, XS, XT)                                                \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulcx_rx(W(XD))

#define mulcx3ld(XD, XS, MT, DT)                                            \
        movcx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movcx_ld(W(XD), W(MT), W(DT))                                       \
        movcx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulcx_rx(W(XD))

#define mulcx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x10))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x10))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x14))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x14))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x18))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x18))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x1C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x1C))                              \
        stack_ld(Recx)                                                      \
        movcx_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_PW8 == 1 */

#define mulcx_rr(XG, XS)                                                    \
        mulcx3rr(W(XG), W(XG), W(XS))

#define mulcx_ld(XG, MS, DS)                                                \
        mulcx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000089 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000089 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000089 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000089 | MXM(RYG(XD), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_PW8 == 1 */

        /* div, rem are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlcx_ri(XG, IS)                                                    \
        shlcx3ri(W(XG), W(XG), W(IS))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlcx3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), TmmM))

#define shlcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcx_ri(XG, IS)                                                    \
        shrcx3ri(W(XG), W(XG), W(IS))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcx3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), TmmM))

#define shrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcn_ri(XG, IS)                                                    \
        shrcn3ri(W(XG), W(XG), W(IS))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcn3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), TmmM))

#define shrcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx3rr(W(XG), W(XG), W(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svlcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx3rr(W(XG), W(XG), W(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn3rr(W(XG), W(XG), W(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcn3rr(XD, XS, XT)                                                \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svrcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), TmmM))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mincx_rr(XG, XS)                                                    \
        mincx3rr(W(XG), W(XG), W(XS))

#define mincx_ld(XG, MS, DS)                                                \
        mincx3ld(W(XG), W(XG), W(MS), W(DS))

#define mincx3rr(XD, XS, XT)                                                \
        EMITW(0x10000282 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000282 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mincx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000282 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000282 | MXM(RYG(XD), RYG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mincn_rr(XG, XS)                                                    \
        mincn3rr(W(XG), W(XG), W(XS))

#define mincn_ld(XG, MS, DS)                                                \
        mincn3ld(W(XG), W(XG), W(MS), W(DS))

#define mincn3rr(XD, XS, XT)                                                \
        EMITW(0x10000382 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000382 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mincn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000382 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000382 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxcx_rr(XG, XS)                                                    \
        maxcx3rr(W(XG), W(XG), W(XS))

#define maxcx_ld(XG, MS, DS)                                                \
        maxcx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000082 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000082 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000082 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000082 | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxcn_rr(XG, XS)                                                    \
        maxcn3rr(W(XG), W(XG), W(XS))

#define maxcn_ld(XG, MS, DS)                                                \
        maxcn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcn3rr(XD, XS, XT)                                                \
        EMITW(0x10000182 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000182 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000182 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000182 | MXM(RYG(XD), RYG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcx_rr(XG, XS)                                                    \
        ceqcx3rr(W(XG), W(XG), W(XS))

#define ceqcx_ld(XG, MS, DS)                                                \
        ceqcx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000086 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000086 | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecx_rr(XG, XS)                                                    \
        cnecx3rr(W(XG), W(XG), W(XS))

#define cnecx_ld(XG, MS, DS)                                                \
        cnecx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecx3rr(XD, XS, XT)                                                \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x10000086 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define cnecx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000086 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltcx_rr(XG, XS)                                                    \
        cltcx3rr(W(XG), W(XG), W(XS))

#define cltcx_ld(XG, MS, DS)                                                \
        cltcx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000286 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltcn_rr(XG, XS)                                                    \
        cltcn3rr(W(XG), W(XG), W(XS))

#define cltcn_ld(XG, MS, DS)                                                \
        cltcn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcn3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000386 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clecx_rr(XG, XS)                                                    \
        clecx3rr(W(XG), W(XG), W(XS))

#define clecx_ld(XG, MS, DS)                                                \
        clecx3ld(W(XG), W(XG), W(MS), W(DS))

#define clecx3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x10000286 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define clecx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clecn_rr(XG, XS)                                                    \
        clecn3rr(W(XG), W(XG), W(XS))

#define clecn_ld(XG, MS, DS)                                                \
        clecn3ld(W(XG), W(XG), W(MS), W(DS))

#define clecn3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x10000386 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define clecn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtcx_rr(XG, XS)                                                    \
        cgtcx3rr(W(XG), W(XG), W(XS))

#define cgtcx_ld(XG, MS, DS)                                                \
        cgtcx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcx3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000286 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(RYG(XD), RYG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtcn_rr(XG, XS)                                                    \
        cgtcn3rr(W(XG), W(XG), W(XS))

#define cgtcn_ld(XG, MS, DS)                                                \
        cgtcn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcn3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000386 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgecx_rr(XG, XS)                                                    \
        cgecx3rr(W(XG), W(XG), W(XS))

#define cgecx_ld(XG, MS, DS)                                                \
        cgecx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecx3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x10000286 | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define cgecx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000286 | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgecn_rr(XG, XS)                                                    \
        cgecn3rr(W(XG), W(XG), W(XS))

#define cgecn_ld(XG, MS, DS)                                                \
        cgecn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecn3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x10000386 | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define cgecn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000386 | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        EMITW(0x10000504 | MXM(RYG(XD), RYG(XD), RYG(XD)))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#define RT_SIMD_WIDTH32_512     16

#define movox2ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(REG(XD), T0xx,    TPxx))                     \
        EMITW(0x7C000619 | MXM(RYG(XD), T1xx,    TPxx))                     \
        EMITW(0x7C000618 | MXM(REG(XD), T2xx,    TPxx))                     \
        EMITW(0x7C000618 | MXM(RYG(XD), T3xx,    TPxx))

#define movox2st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(REG(XS), T0xx,    TPxx))                     \
        EMITW(0x7C000719 | MXM(RYG(XS), T1xx,    TPxx))                     \
        EMITW(0x7C000718 | MXM(REG(XS), T2xx,    TPxx))                     \
        EMITW(0x7C000718 | MXM(RYG(XS), T3xx,    TPxx))

/* sregs */

#undef  sregs_sa
#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox2st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        EMITW(0x7C000719 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C000719 | MXM(TmmM,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C000718 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C000718 | MXM(TmmM,    0x00,    TEax))

#undef  sregs_la
#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox2ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        movox2ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_512*4))                           \
        EMITW(0x7C000619 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C000619 | MXM(TmmM,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C000618 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x7C000618 | MXM(TmmM,    0x00,    TEax))

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

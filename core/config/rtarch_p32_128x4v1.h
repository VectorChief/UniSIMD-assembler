/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_128X4V1_H
#define RT_RTARCH_P32_128X4V1_H

#include "rtarch_p64.h"

#define RT_SIMD_REGS            16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_128x4v1.h: Implementation of POWER fp32 VSX1/2 instruction quads.
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

#if (RT_128X4 == 1) && (RT_SIMD_COMPAT_XMM > 0)

#ifndef RT_RTARCH_P64_128X2V1_H
#undef  RT_128X2
#define RT_128X2  RT_128X4
#include "rtarch_p64_128x2v1.h"
#endif /* RT_RTARCH_P64_128X2V1_H */

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
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000490 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000490 | MXM(RYG(XD), RYG(XS), RYG(XS)))

#define movox_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(REG(XD), T0xx,    TPxx))                     \
        EMITW(0x7C000619 | MXM(RYG(XD), T1xx,    TPxx))                     \
        EMITW(0x7C000618 | MXM(REG(XD), T2xx,    TPxx))                     \
        EMITW(0x7C000618 | MXM(RYG(XD), T3xx,    TPxx))

#define movox_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C000719 | MXM(REG(XS), T0xx,    TPxx))                     \
        EMITW(0x7C000719 | MXM(RYG(XS), T1xx,    TPxx))                     \
        EMITW(0x7C000718 | MXM(REG(XS), T2xx,    TPxx))                     \
        EMITW(0x7C000718 | MXM(RYG(XS), T3xx,    TPxx))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvox_rr(XG, XS)                                                    \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF0000030 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF0000430 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#define mmvox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000003F | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000043F | MXM(RYG(XG), RYG(XG), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000032 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000432 | MXM(RYG(XG), RYG(XG), TmmM))

#define mmvox_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000003F | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000043F | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000035 | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000435 | MXM(TmmM,    TmmM,    RYG(XS)))                  \
        EMITW(0x7C000719 | MXM(TmmM,    T3xx,    TPxx))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andox_rr(XG, XS)                                                    \
        andox3rr(W(XG), W(XG), W(XS))

#define andox_ld(XG, MS, DS)                                                \
        andox3ld(W(XG), W(XG), W(MS), W(DS))

#define andox3rr(XD, XS, XT)                                                \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000410 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000410 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000417 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000417 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000412 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000412 | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annox_rr(XG, XS)                                                    \
        annox3rr(W(XG), W(XG), W(XS))

#define annox_ld(XG, MS, DS)                                                \
        annox3ld(W(XG), W(XG), W(MS), W(DS))

#define annox3rr(XD, XS, XT)                                                \
        EMITW(0xF0000457 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000457 | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000450 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000450 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define annox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000457 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000457 | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000454 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000454 | MXM(RYG(XD), TmmM,    RYG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrox_rr(XG, XS)                                                    \
        orrox3rr(W(XG), W(XG), W(XS))

#define orrox_ld(XG, MS, DS)                                                \
        orrox3ld(W(XG), W(XG), W(MS), W(DS))

#define orrox3rr(XD, XS, XT)                                                \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000490 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000490 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000497 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000497 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000492 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000492 | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#if (RT_SIMD_COMPAT_PW8 == 0)

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

#else /* RT_SIMD_COMPAT_PW8 == 1 */

#define ornox_rr(XG, XS)                                                    \
        ornox3rr(W(XG), W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        ornox3ld(W(XG), W(XG), W(MS), W(DS))

#define ornox3rr(XD, XS, XT)                                                \
        EMITW(0xF0000557 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000557 | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000550 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000550 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define ornox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000557 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000557 | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000554 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000554 | MXM(RYG(XD), TmmM,    RYG(XS)))

#endif /* RT_SIMD_COMPAT_PW8 == 1 */

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorox_rr(XG, XS)                                                    \
        xorox3rr(W(XG), W(XG), W(XS))

#define xorox_ld(XG, MS, DS)                                                \
        xorox3ld(W(XG), W(XG), W(MS), W(DS))

#define xorox3rr(XD, XS, XT)                                                \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF00004D0 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00004D0 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xorox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00004D7 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00004D7 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00004D2 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00004D2 | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notox_rx(XG)                                                        \
        notox_rr(W(XG), W(XG))

#define notox_rr(XD, XS)                                                    \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000510 | MXM(REG(XD), REG(XS), REG(XS)))                  \
        EMITW(0xF0000510 | MXM(RYG(XD), RYG(XS), RYG(XS)))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negos_rx(XG)                                                        \
        negos_rr(W(XG), W(XG))

#define negos_rr(XD, XS)                                                    \
        EMITW(0xF00006E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00006E7 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00006E4 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00006E4 | MXM(RYG(XD), 0x00,    RYG(XS)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addos_rr(XG, XS)                                                    \
        addos3rr(W(XG), W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addos3ld(W(XG), W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        EMITW(0xF0000207 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000207 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000200 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000200 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000207 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000207 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000202 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000202 | MXM(RYG(XD), RYG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subos_rr(XG, XS)                                                    \
        subos3rr(W(XG), W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subos3ld(W(XG), W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        EMITW(0xF0000247 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000247 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000240 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000240 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000247 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000247 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000242 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000242 | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulos_rr(XG, XS)                                                    \
        mulos3rr(W(XG), W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulos3ld(W(XG), W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        EMITW(0xF0000287 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000287 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000280 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000280 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000287 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000287 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000282 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000282 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divos_rr(XG, XS)                                                    \
        divos3rr(W(XG), W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divos3ld(W(XG), W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        EMITW(0xF00002C7 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00002C7 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF00002C0 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF00002C0 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002C7 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002C7 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002C2 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002C2 | MXM(RYG(XD), RYG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF000022C | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022C | MXM(RYG(XD), 0x00,    RYG(XS)))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000022F | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000022F | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000022E | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000022E | MXM(RYG(XD), 0x00,    TmmM))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        EMITW(0xF000026B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000026B | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000268 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000268 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF00006CD | MXM(REG(XS), REG(XG), TmmQ))                     \
        EMITW(0xF000020F | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00006CD | MXM(RYG(XS), RYG(XG), TmmQ))                     \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XG), RYG(XS)))                  \
        EMITW(0xF00006C8 | MXM(REG(XS), REG(XG), TmmQ))                     \
        EMITW(0xF0000208 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0xF00006C8 | MXM(RYG(XS), RYG(XG), TmmQ))                     \
        EMITW(0xF0000208 | MXM(RYG(XG), RYG(XG), RYG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        EMITW(0xF000022B | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF000022B | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000228 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000228 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0xF0000287 | MXM(TmmM,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000285 | MXM(TmmQ,    REG(XG), TmmM))                     \
        EMITW(0xF00006CD | MXM(TmmM,    REG(XS), TmmQ))                     \
        EMITW(0xF000068F | MXM(REG(XG), TmmM,    TmmQ))                     \
        EMITW(0xF0000287 | MXM(TmmM,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000285 | MXM(TmmQ,    RYG(XG), TmmM))                     \
        EMITW(0xF00006CD | MXM(TmmM,    RYG(XS), TmmQ))                     \
        EMITW(0xF000068F | MXM(RYG(XG), TmmM,    TmmQ))                     \
        EMITW(0xF0000281 | MXM(TmmM,    REG(XG), REG(XG)))                  \
        EMITW(0xF0000281 | MXM(TmmQ,    REG(XG), TmmM))                     \
        EMITW(0xF00006C9 | MXM(TmmM,    REG(XS), TmmQ))                     \
        EMITW(0xF000068E | MXM(REG(XG), TmmM,    TmmQ))                     \
        EMITW(0xF0000281 | MXM(TmmM,    RYG(XG), RYG(XG)))                  \
        EMITW(0xF0000281 | MXM(TmmQ,    RYG(XG), TmmM))                     \
        EMITW(0xF00006C9 | MXM(TmmM,    RYG(XS), TmmQ))                     \
        EMITW(0xF000068E | MXM(RYG(XG), TmmM,    TmmQ))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000208 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF0000208 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000020F | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000020F | MXM(RYG(XG), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000020A | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000020A | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000688 | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0xF0000688 | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000068F | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000068F | MXM(RYG(XG), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000068A | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000068A | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minos_rr(XG, XS)                                                    \
        minos3rr(W(XG), W(XG), W(XS))

#define minos_ld(XG, MS, DS)                                                \
        minos3ld(W(XG), W(XG), W(MS), W(DS))

#define minos3rr(XD, XS, XT)                                                \
        EMITW(0xF0000647 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000647 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000640 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000640 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define minos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000647 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000647 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000642 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000642 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxos_rr(XG, XS)                                                    \
        maxos3rr(W(XG), W(XG), W(XS))

#define maxos_ld(XG, MS, DS)                                                \
        maxos3ld(W(XG), W(XG), W(MS), W(DS))

#define maxos3rr(XD, XS, XT)                                                \
        EMITW(0xF0000607 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000607 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000600 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000600 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000607 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000607 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000602 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000602 | MXM(RYG(XD), RYG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqos_rr(XG, XS)                                                    \
        ceqos3rr(W(XG), W(XG), W(XS))

#define ceqos_ld(XG, MS, DS)                                                \
        ceqos3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqos3rr(XD, XS, XT)                                                \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000218 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000218 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000021A | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000021A | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneos_rr(XG, XS)                                                    \
        cneos3rr(W(XG), W(XG), W(XS))

#define cneos_ld(XG, MS, DS)                                                \
        cneos3ld(W(XG), W(XG), W(MS), W(DS))

#define cneos3rr(XD, XS, XT)                                                \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0xF0000218 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000510 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0xF0000218 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000510 | MXM(RYG(XD), RYG(XD), RYG(XD)))

#define cneos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0xF0000517 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000021F | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000517 | MXM(RYG(XD), RYG(XD), RYG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000021A | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0xF0000510 | MXM(REG(XD), REG(XD), REG(XD)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000021A | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000510 | MXM(RYG(XD), RYG(XD), RYG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltos_rr(XG, XS)                                                    \
        cltos3rr(W(XG), W(XG), W(XS))

#define cltos_ld(XG, MS, DS)                                                \
        cltos3ld(W(XG), W(XG), W(MS), W(DS))

#define cltos3rr(XD, XS, XT)                                                \
        EMITW(0xF000025F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF000025F | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000258 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000258 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cltos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000025C | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000025C | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleos_rr(XG, XS)                                                    \
        cleos3rr(W(XG), W(XG), W(XS))

#define cleos_ld(XG, MS, DS)                                                \
        cleos3ld(W(XG), W(XG), W(MS), W(DS))

#define cleos3rr(XD, XS, XT)                                                \
        EMITW(0xF000029F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF000029F | MXM(RYG(XD), RYG(XT), RYG(XS)))                  \
        EMITW(0xF0000298 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0xF0000298 | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cleos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(RYG(XD), TmmM,    RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000029C | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000029C | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtos_rr(XG, XS)                                                    \
        cgtos3rr(W(XG), W(XG), W(XS))

#define cgtos_ld(XG, MS, DS)                                                \
        cgtos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtos3rr(XD, XS, XT)                                                \
        EMITW(0xF000025F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000025F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000258 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000258 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgtos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000025F | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000025A | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000025A | MXM(RYG(XD), RYG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeos_rr(XG, XS)                                                    \
        cgeos3rr(W(XG), W(XG), W(XS))

#define cgeos_ld(XG, MS, DS)                                                \
        cgeos3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeos3rr(XD, XS, XT)                                                \
        EMITW(0xF000029F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF000029F | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000298 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0xF0000298 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cgeos3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF000029F | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF000029A | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF000029A | MXM(RYG(XD), RYG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_512  MN32_512   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_512  MF32_512   /*  all satisfy the condition */

/* #define S0(mask)    S1(mask)            (defined in 32_128-bit header) */
/* #define S1(mask)    S##mask             (defined in 32_128-bit header) */

#define SMN32_512(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000497 | MXM(TmmM,    xs,      xs+16))                    \
        EMITW(0xF0000491 | MXM(TmmQ,    xs,      xs+16))                    \
        EMITW(0xF0000497 | MXM(TmmM,    TmmM,    TmmQ))                     \
        EMITW(0x1000038C | MXM(TmmQ,    0x1F,    0x00))                     \
        EMITW(0x10000486 | MXM(TmmM,    TmmM,    TmmQ))                     \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF32_512(xs, lb) /* not portable, do not use outside */            \
        EMITW(0xF0000417 | MXM(TmmM,    xs,      xs+16))                    \
        EMITW(0xF0000411 | MXM(TmmQ,    xs,      xs+16))                    \
        EMITW(0xF0000417 | MXM(TmmM,    TmmM,    TmmQ))                     \
        EMITW(0x1000038C | MXM(TmmQ,    0x1F,    0x00))                     \
        EMITW(0x10000486 | MXM(TmmM,    TmmM,    TmmQ))                     \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##32_512), EMPTY2)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000264 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000264 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000266 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000266 | MXM(RYG(XD), 0x00,    TmmM))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000260 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000260 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000263 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000263 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000262 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000262 | MXM(RYG(XD), 0x00,    TmmM))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002A4 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A4 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002A6 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002A6 | MXM(RYG(XD), 0x00,    TmmM))

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
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002E4 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E4 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002E6 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002E6 | MXM(RYG(XD), 0x00,    TmmM))

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
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002AC | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AC | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(RYG(XD), 0x00,    TmmM))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        rnnos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        rnnos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002AC | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AC | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(RYG(XD), 0x00,    TmmM))

#define cvtos_rr(XD, XS)                                                    \
        rndos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvtos_ld(XD, MS, DS)                                                \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0x1000020A | MXM(RYG(XD), 0x00,    RYG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XD), REG(XD)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XS), REG(XS)))                  \
        EMITW(0x1000020A | MXM(TmmQ,    0x00,    TmmM)    |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XD), RYG(XD)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XS), RYG(XS)))                  \
        EMITW(0x1000020A | MXM(TmmQ,    0x00,    TmmM)    |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define cvros_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)                                        \
        cvzos_rr(W(XD), W(XD))

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
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002E0 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E0 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvton_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002E3 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002E3 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002E2 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002E2 | MXM(RYG(XD), 0x00,    TmmM))

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
        EMITW(0xF00002A3 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A3 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002A0 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A0 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cvtox_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002A3 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002A3 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002A2 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002A2 | MXM(RYG(XD), 0x00,    TmmM))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000264 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000264 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define ruzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000267 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000266 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000266 | MXM(RYG(XD), 0x00,    TmmM))

#define cuzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0xF0000223 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000223 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF0000220 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF0000220 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define cuzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF0000223 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF0000223 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF0000222 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF0000222 | MXM(RYG(XD), 0x00,    TmmM))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupos_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002A4 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002A4 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rupos_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002A7 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002A6 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002A6 | MXM(RYG(XD), 0x00,    TmmM))

#define cupos_rr(XD, XS)     /* round towards +inf */                       \
        rupos_rr(W(XD), W(XS))                                              \
        cuzos_rr(W(XD), W(XD))

#define cupos_ld(XD, MS, DS) /* round towards +inf */                       \
        rupos_ld(W(XD), W(MS), W(DS))                                       \
        cuzos_rr(W(XD), W(XD))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumos_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002E4 | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002E4 | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rumos_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002E7 | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002E6 | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002E6 | MXM(RYG(XD), 0x00,    TmmM))

#define cumos_rr(XD, XS)     /* round towards -inf */                       \
        rumos_rr(W(XD), W(XS))                                              \
        cuzos_rr(W(XD), W(XD))

#define cumos_ld(XD, MS, DS) /* round towards -inf */                       \
        rumos_ld(W(XD), W(MS), W(DS))                                       \
        cuzos_rr(W(XD), W(XD))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runos_rr(XD, XS)     /* round towards near */                       \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002AC | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AC | MXM(RYG(XD), 0x00,    RYG(XS)))

#define runos_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(RYG(XD), 0x00,    TmmM))

#define cunos_rr(XD, XS)     /* round towards near */                       \
        runos_rr(W(XD), W(XS))                                              \
        cuzos_rr(W(XD), W(XD))

#define cunos_ld(XD, MS, DS) /* round towards near */                       \
        runos_ld(W(XD), W(MS), W(DS))                                       \
        cuzos_rr(W(XD), W(XD))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudos_rr(XD, XS)                                                    \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    RYG(XS)))                  \
        EMITW(0xF00002AC | MXM(REG(XD), 0x00,    REG(XS)))                  \
        EMITW(0xF00002AC | MXM(RYG(XD), 0x00,    RYG(XS)))

#define rudos_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0xF00002AF | MXM(RYG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(REG(XD), 0x00,    TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0xF00002AE | MXM(RYG(XD), 0x00,    TmmM))

#define cutos_rr(XD, XS)                                                    \
        rudos_rr(W(XD), W(XS))                                              \
        cuzos_rr(W(XD), W(XD))

#define cutos_ld(XD, MS, DS)                                                \
        rudos_ld(W(XD), W(MS), W(DS))                                       \
        cuzos_rr(W(XD), W(XD))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruros_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0x1000020A | MXM(RYG(XD), 0x00,    RYG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XD), REG(XD)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XS), REG(XS)))                  \
        EMITW(0x1000020A | MXM(TmmQ,    0x00,    TmmM)    |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XD), RYG(XD)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XS), RYG(XS)))                  \
        EMITW(0x1000020A | MXM(TmmQ,    0x00,    TmmM)    |                 \
        (RT_SIMD_MODE_##mode&3) << 6)                                       \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define curos_rr(XD, XS, mode)                                              \
        ruros_rr(W(XD), W(XS), mode)                                        \
        cuzos_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addox_rr(XG, XS)                                                    \
        addox3rr(W(XG), W(XG), W(XS))

#define addox_ld(XG, MS, DS)                                                \
        addox3ld(W(XG), W(XG), W(MS), W(DS))

#define addox3rr(XD, XS, XT)                                                \
        EMITW(0x10000080 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000080 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XT), REG(XT)))                  \
        EMITW(0x10000080 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XT), RYG(XT)))                  \
        EMITW(0x10000080 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define addox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000080 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000080 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0x10000080 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0x10000080 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subox_rr(XG, XS)                                                    \
        subox3rr(W(XG), W(XG), W(XS))

#define subox_ld(XG, MS, DS)                                                \
        subox3ld(W(XG), W(XG), W(MS), W(DS))

#define subox3rr(XD, XS, XT)                                                \
        EMITW(0x10000480 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000480 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XT), REG(XT)))                  \
        EMITW(0x10000480 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XT), RYG(XT)))                  \
        EMITW(0x10000480 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define subox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000480 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000480 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0x10000480 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0x10000480 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#if (RT_SIMD_COMPAT_PW8 == 0)

#define mulox_rr(XG, XS)                                                    \
        mulox3rr(W(XG), W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulox3ld(W(XG), W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulox_rx(W(XD))

#define mulox3ld(XD, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XD), W(MT), W(DT))                                       \
        movox_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulox_rx(W(XD))

#define mulox_rx(XD) /* not portable, do not use outside */                 \
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
        movwx_ld(Recx,  Mebp, inf_SCR01(0x20))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x20))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x24))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x24))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x28))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x28))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x2C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x2C))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x30))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x30))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x34))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x34))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x38))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x38))                              \
        movwx_ld(Recx,  Mebp, inf_SCR01(0x3C))                              \
        mulwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        movwx_st(Recx,  Mebp, inf_SCR01(0x3C))                              \
        stack_ld(Recx)                                                      \
        movox_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_PW8 == 1 */

#define mulox_rr(XG, XS)                                                    \
        mulox3rr(W(XG), W(XG), W(XS))

#define mulox_ld(XG, MS, DS)                                                \
        mulox3ld(W(XG), W(XG), W(MS), W(DS))

#define mulox3rr(XD, XS, XT)                                                \
        EMITW(0x10000089 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000089 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XT), REG(XT)))                  \
        EMITW(0x10000089 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XT), RYG(XT)))                  \
        EMITW(0x10000089 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define mulox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000089 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000089 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0x10000089 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0x10000089 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#endif /* RT_SIMD_COMPAT_PW8 == 1 */

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlox_ri(XG, IS)                                                    \
        shlox3ri(W(XG), W(XG), W(IS))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlox3ld(W(XG), W(XG), W(MS), W(DS))

#define shlox3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define shlox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrox_ri(XG, IS)                                                    \
        shrox3ri(W(XG), W(XG), W(IS))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrox3ld(W(XG), W(XG), W(MS), W(DS))

#define shrox3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define shrox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shron_ri(XG, IS)                                                    \
        shron3ri(W(XG), W(XG), W(IS))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shron3ld(W(XG), W(XG), W(MS), W(DS))

#define shron3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define shron3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlox3rr(W(XG), W(XG), W(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlox3ld(W(XG), W(XG), W(MS), W(DS))

#define svlox3rr(XD, XS, XT)                                                \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XT), REG(XT)))                  \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XT), RYG(XT)))                  \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define svlox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000184 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0x10000184 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrox3rr(W(XG), W(XG), W(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrox3ld(W(XG), W(XG), W(MS), W(DS))

#define svrox3rr(XD, XS, XT)                                                \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XT), REG(XT)))                  \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XT), RYG(XT)))                  \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define svrox3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000284 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0x10000284 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        svron3rr(W(XG), W(XG), W(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svron3ld(W(XG), W(XG), W(MS), W(DS))

#define svron3rr(XD, XS, XT)                                                \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), RYG(XT)))                  \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    REG(XT), REG(XT)))                  \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0xF0000491 | MXM(TmmM,    RYG(XT), RYG(XT)))                  \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

#define svron3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C000619 | MXM(TmmM,    T0xx,    TPxx))                     \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7C000619 | MXM(TmmM,    T1xx,    TPxx))                     \
        EMITW(0x10000384 | MXM(RYG(XD), RYG(XS), TmmM))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    REG(XS), REG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T2xx,    TPxx))                     \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(REG(XD), TmmQ,    TmmQ))                     \
        EMITW(0xF0000491 | MXM(TmmQ,    RYG(XS), RYG(XS)))                  \
        EMITW(0x7C000619 | MXM(TmmM,    T3xx,    TPxx))                     \
        EMITW(0x10000384 | MXM(TmmQ,    TmmQ,    TmmM))                     \
        EMITW(0xF0000496 | MXM(RYG(XD), TmmQ,    TmmQ))

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
        EMITW(0x7C000719 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000719 | MXM(TmmM,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000718 | MXM(TmmM,    0x00,    TEax))

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
        EMITW(0x7C000619 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000619 | MXM(TmmM,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmQ,    0x00,    TEax))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x7C000618 | MXM(TmmM,    0x00,    TEax))

#endif /* RT_128X4 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_128X4V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

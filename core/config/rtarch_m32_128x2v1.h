/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_128X2V1_H
#define RT_RTARCH_M32_128X2V1_H

#include "rtarch_m64.h"

#define RT_SIMD_REGS_256        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32_128x2v1.h: Implementation of MIPS32 fp32 MSA instruction pairs.
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

#if (RT_128X2 != 0) && (RT_SIMD_COMPAT_XMM > 0)

#ifndef RT_RTARCH_M64_128X1V1_H
#undef  RT_128X1
#define RT_128X1  RT_128X2
#include "rtarch_m64_128x1v1.h"
#endif /* RT_RTARCH_M64_128X1V1_H */

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
        EMITW(0x78BE0019 | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x78BE0019 | MXM(RYG(XD), RYG(XS), 0x00))

#define movcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(REG(XD), MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(REG(XD), REG(XD), 0x00)))                    \
        EMITW(0x78000022 | MFM(RYG(XD), MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(RYG(XD), RYG(XD), 0x00)))

#define movcx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A2(DD), EMPTY2)   \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    REG(XS), 0x00)))                    \
    SHF(EMITW(0x78000026 | MFM(TmmM,    MOD(MD), VAL(DD), B4(DD), K2(DD)))) \
    SHX(EMITW(0x78000026 | MFM(REG(XS), MOD(MD), VAL(DD), B4(DD), K2(DD)))) \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    RYG(XS), 0x00)))                    \
    SJF(EMITW(0x78000026 | MFM(TmmM,    MOD(MD), VYL(DD), B4(DD), K2(DD)))) \
    SJX(EMITW(0x78000026 | MFM(RYG(XS), MOD(MD), VYL(DD), B4(DD), K2(DD))))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvcx_rr(XG, XS)                                                    \
        EMITW(0x7880001E | MXM(REG(XG), REG(XS), Tmm0))                     \
        EMITW(0x7880001E | MXM(RYG(XG), RYG(XS), Tmm0+16))

#define mmvcx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001E | MXM(REG(XG), TmmM,    Tmm0))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001E | MXM(RYG(XG), TmmM,    Tmm0+16))

#define mmvcx_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A2(DG), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MG), VAL(DG), B4(DG), K2(DG)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001E | MXM(TmmM,    REG(XS), Tmm0))                     \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78000026 | MFM(TmmM,    MOD(MG), VAL(DG), B4(DG), K2(DG)))  \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MG), VYL(DG), B4(DG), K2(DG)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001E | MXM(TmmM,    RYG(XS), Tmm0+16))                  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78000026 | MFM(TmmM,    MOD(MG), VYL(DG), B4(DG), K2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andcx_rr(XG, XS)                                                    \
        andcx3rr(W(XG), W(XG), W(XS))

#define andcx_ld(XG, MS, DS)                                                \
        andcx3ld(W(XG), W(XG), W(MS), W(DS))

#define andcx3rr(XD, XS, XT)                                                \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7800001E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define andcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7800001E | MXM(RYG(XD), RYG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anncx_rr(XG, XS)                                                    \
        EMITW(0x78C0001E | MXM(REG(XG), REG(XS), TmmZ))                     \
        EMITW(0x78C0001E | MXM(RYG(XG), RYG(XS), TmmZ))

#define anncx_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001E | MXM(REG(XG), TmmM,    TmmZ))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001E | MXM(RYG(XG), TmmM,    TmmZ))

#define anncx3rr(XD, XS, XT)                                                \
        movcx_rr(W(XD), W(XS))                                              \
        anncx_rr(W(XD), W(XT))

#define anncx3ld(XD, XS, MT, DT)                                            \
        movcx_rr(W(XD), W(XS))                                              \
        anncx_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrcx_rr(XG, XS)                                                    \
        orrcx3rr(W(XG), W(XG), W(XS))

#define orrcx_ld(XG, MS, DS)                                                \
        orrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrcx3rr(XD, XS, XT)                                                \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7820001E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define orrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7820001E | MXM(RYG(XD), RYG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorcx_rr(XG, XS)                                                    \
        xorcx3rr(W(XG), W(XG), W(XS))

#define xorcx_ld(XG, MS, DS)                                                \
        xorcx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorcx3rr(XD, XS, XT)                                                \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7860001E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define xorcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7860001E | MXM(RYG(XD), RYG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notcx_rx(XG)                                                        \
        notcx_rr(W(XG), W(XG))

#define notcx_rr(XD, XS)                                                    \
        EMITW(0x7840001E | MXM(REG(XD), TmmZ,    REG(XS)))                  \
        EMITW(0x7840001E | MXM(RYG(XD), TmmZ,    RYG(XS)))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negcs_rx(XG)                                                        \
        negcs_rr(W(XG), W(XG))

#define negcs_rr(XD, XS)                                                    \
        movix_xm(Mebp, inf_GPC06_32)                                        \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7860001E | MXM(RYG(XD), RYG(XS), TmmM))

/* #define movix_xm(MS, DS)                (defined in 32_128-bit header) */

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcs_rr(XG, XS)                                                    \
        addcs3rr(W(XG), W(XG), W(XS))

#define addcs_ld(XG, MS, DS)                                                \
        addcs3ld(W(XG), W(XG), W(MS), W(DS))

#define addcs3rr(XD, XS, XT)                                                \
        EMITW(0x7800001B | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7800001B | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7800001B | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7800001B | MXM(RYG(XD), RYG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcs_rr(XG, XS)                                                    \
        subcs3rr(W(XG), W(XG), W(XS))

#define subcs_ld(XG, MS, DS)                                                \
        subcs3ld(W(XG), W(XG), W(MS), W(DS))

#define subcs3rr(XD, XS, XT)                                                \
        EMITW(0x7840001B | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7840001B | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840001B | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840001B | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcs_rr(XG, XS)                                                    \
        mulcs3rr(W(XG), W(XG), W(XS))

#define mulcs_ld(XG, MS, DS)                                                \
        mulcs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcs3rr(XD, XS, XT)                                                \
        EMITW(0x7880001B | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7880001B | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001B | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001B | MXM(RYG(XD), RYG(XS), TmmM))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divcs_rr(XG, XS)                                                    \
        divcs3rr(W(XG), W(XG), W(XS))

#define divcs_ld(XG, MS, DS)                                                \
        divcs3ld(W(XG), W(XG), W(MS), W(DS))

#define divcs3rr(XD, XS, XT)                                                \
        EMITW(0x78C0001B | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78C0001B | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001B | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001B | MXM(RYG(XD), RYG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrcs_rr(XD, XS)                                                    \
        EMITW(0x7B26001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B26001E | MXM(RYG(XD), RYG(XS), 0x00))

#define sqrcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B26001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B26001E | MXM(RYG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcecs_rr(XD, XS)                                                    \
        EMITW(0x7B2A001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B2A001E | MXM(RYG(XD), RYG(XS), 0x00))

#define rcscs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsecs_rr(XD, XS)                                                    \
        EMITW(0x7B28001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B28001E | MXM(RYG(XD), RYG(XS), 0x00))

#define rsscs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmacs_rr(XG, XS, XT)                                                \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x7900001B | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmacs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001B | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmscs_rr(XG, XS, XT)                                                \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), REG(XT)))                  \
        EMITW(0x7940001B | MXM(RYG(XG), RYG(XS), RYG(XT)))

#define fmscs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940001B | MXM(RYG(XG), RYG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define mincs_rr(XG, XS)                                                    \
        mincs3rr(W(XG), W(XG), W(XS))

#define mincs_ld(XG, MS, DS)                                                \
        mincs3ld(W(XG), W(XG), W(MS), W(DS))

#define mincs3rr(XD, XS, XT)                                                \
        EMITW(0x7B00001B | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7B00001B | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mincs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B00001B | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B00001B | MXM(RYG(XD), RYG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxcs_rr(XG, XS)                                                    \
        maxcs3rr(W(XG), W(XG), W(XS))

#define maxcs_ld(XG, MS, DS)                                                \
        maxcs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcs3rr(XD, XS, XT)                                                \
        EMITW(0x7B80001B | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7B80001B | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B80001B | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B80001B | MXM(RYG(XD), RYG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcs_rr(XG, XS)                                                    \
        ceqcs3rr(W(XG), W(XG), W(XS))

#define ceqcs_ld(XG, MS, DS)                                                \
        ceqcs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcs3rr(XD, XS, XT)                                                \
        EMITW(0x7880001A | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7880001A | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001A | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001A | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecs_rr(XG, XS)                                                    \
        cnecs3rr(W(XG), W(XG), W(XS))

#define cnecs_ld(XG, MS, DS)                                                \
        cnecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecs3rr(XD, XS, XT)                                                \
        EMITW(0x78C0001C | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78C0001C | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cnecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001C | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001C | MXM(RYG(XD), RYG(XS), TmmM))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltcs_rr(XG, XS)                                                    \
        cltcs3rr(W(XG), W(XG), W(XS))

#define cltcs_ld(XG, MS, DS)                                                \
        cltcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcs3rr(XD, XS, XT)                                                \
        EMITW(0x7900001A | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7900001A | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cltcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001A | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001A | MXM(RYG(XD), RYG(XS), TmmM))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clecs_rr(XG, XS)                                                    \
        clecs3rr(W(XG), W(XG), W(XS))

#define clecs_ld(XG, MS, DS)                                                \
        clecs3ld(W(XG), W(XG), W(MS), W(DS))

#define clecs3rr(XD, XS, XT)                                                \
        EMITW(0x7980001A | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7980001A | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define clecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7980001A | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7980001A | MXM(RYG(XD), RYG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtcs_rr(XG, XS)                                                    \
        cgtcs3rr(W(XG), W(XG), W(XS))

#define cgtcs_ld(XG, MS, DS)                                                \
        cgtcs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcs3rr(XD, XS, XT)                                                \
        EMITW(0x7900001A | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x7900001A | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cgtcs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001A | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001A | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgecs_rr(XG, XS)                                                    \
        cgecs3rr(W(XG), W(XG), W(XS))

#define cgecs_ld(XG, MS, DS)                                                \
        cgecs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecs3rr(XD, XS, XT)                                                \
        EMITW(0x7980001A | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x7980001A | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cgecs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7980001A | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7980001A | MXM(RYG(XD), TmmM,    RYG(XS)))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_256  MN32_256   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_256  MF32_256   /*  all satisfy the condition */

/* #define S0(mask)    S1(mask)            (defined in 32_128-bit header) */
/* #define S1(mask)    S##mask             (defined in 32_128-bit header) */

#define SMN32_256(xs, lb) /* not portable, do not use outside */            \
        EMITW(0x7820001E | MXM(TmmM, xs, xs+16))                            \
        ASM_BEG ASM_OP2( bz.v, $w31, lb) ASM_END

#define SMF32_256(xs, lb) /* not portable, do not use outside */            \
        EMITW(0x7800001E | MXM(TmmM, xs, xs+16))                            \
        ASM_BEG ASM_OP2(bnz.w, $w31, lb) ASM_END

#define mkjcx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, REG(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##32_256), EMPTY2)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzcs_rr(XD, XS)     /* round towards zero */                       \
        cvzcs_rr(W(XD), W(XS))                                              \
        cvncn_rr(W(XD), W(XD))

#define rnzcs_ld(XD, MS, DS) /* round towards zero */                       \
        cvzcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncn_rr(W(XD), W(XD))

#define cvzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x7B22001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B22001E | MXM(RYG(XD), RYG(XS), 0x00))

#define cvzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B22001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B22001E | MXM(RYG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpcs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rnpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cvpcs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpcs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmcs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rnmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cvmcs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmcs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnncs_rr(XD, XS)     /* round towards near */                       \
        rndcs_rr(W(XD), W(XS))

#define rnncs_ld(XD, MS, DS) /* round towards near */                       \
        rndcs_ld(W(XD), W(MS), W(DS))

#define cvncs_rr(XD, XS)     /* round towards near */                       \
        cvtcs_rr(W(XD), W(XS))

#define cvncs_ld(XD, MS, DS) /* round towards near */                       \
        cvtcs_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndcs_rr(XD, XS)                                                    \
        EMITW(0x7B2C001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B2C001E | MXM(RYG(XD), RYG(XS), 0x00))

#define rndcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B2C001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B2C001E | MXM(RYG(XD), TmmM,    0x00))

#define cvtcs_rr(XD, XS)                                                    \
        EMITW(0x7B38001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B38001E | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B38001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B38001E | MXM(RYG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnrcs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvrcs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

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
        EMITW(0x7B3C001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B3C001E | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtcn_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B3C001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B3C001E | MXM(RYG(XD), TmmM,    0x00))

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
        EMITW(0x7B3E001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B3E001E | MXM(RYG(XD), RYG(XS), 0x00))

#define cvtcx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B3E001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B3E001E | MXM(RYG(XD), TmmM,    0x00))

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzcs_rr(XD, XS)     /* round towards zero */                       \
        cuzcs_rr(W(XD), W(XS))                                              \
        cvncx_rr(W(XD), W(XD))

#define ruzcs_ld(XD, MS, DS) /* round towards zero */                       \
        cuzcs_ld(W(XD), W(MS), W(DS))                                       \
        cvncx_rr(W(XD), W(XD))

#define cuzcs_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x7B24001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B24001E | MXM(RYG(XD), RYG(XS), 0x00))

#define cuzcs_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B24001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B24001E | MXM(RYG(XD), TmmM,    0x00))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupcs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rudcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rudcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cupcs_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cutcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cupcs_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cutcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumcs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rudcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rudcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cumcs_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cutcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cumcs_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cutcs_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runcs_rr(XD, XS)     /* round towards near */                       \
        rudcs_rr(W(XD), W(XS))

#define runcs_ld(XD, MS, DS) /* round towards near */                       \
        rudcs_ld(W(XD), W(MS), W(DS))

#define cuncs_rr(XD, XS)     /* round towards near */                       \
        cutcs_rr(W(XD), W(XS))

#define cuncs_ld(XD, MS, DS) /* round towards near */                       \
        cutcs_ld(W(XD), W(MS), W(DS))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudcs_rr(XD, XS)                                                    \
        EMITW(0x7B2C001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B2C001E | MXM(RYG(XD), RYG(XS), 0x00))

#define rudcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B2C001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B2C001E | MXM(RYG(XD), TmmM,    0x00))

#define cutcs_rr(XD, XS)                                                    \
        EMITW(0x7B3A001E | MXM(REG(XD), REG(XS), 0x00))                     \
        EMITW(0x7B3A001E | MXM(RYG(XD), RYG(XS), 0x00))

#define cutcs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), K2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B3A001E | MXM(REG(XD), TmmM,    0x00))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VYL(DS), B4(DS), K2(DS)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B3A001E | MXM(RYG(XD), TmmM,    0x00))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rurcs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rudcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define curcs_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cutcs_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addcx_rr(XG, XS)                                                    \
        addcx3rr(W(XG), W(XG), W(XS))

#define addcx_ld(XG, MS, DS)                                                \
        addcx3ld(W(XG), W(XG), W(MS), W(DS))

#define addcx3rr(XD, XS, XT)                                                \
        EMITW(0x7840000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7840000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define addcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000E | MXM(RYG(XD), RYG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subcx_rr(XG, XS)                                                    \
        subcx3rr(W(XG), W(XG), W(XS))

#define subcx_ld(XG, MS, DS)                                                \
        subcx3ld(W(XG), W(XG), W(MS), W(DS))

#define subcx3rr(XD, XS, XT)                                                \
        EMITW(0x78C0000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78C0000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define subcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0000E | MXM(RYG(XD), RYG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulcx_rr(XG, XS)                                                    \
        mulcx3rr(W(XG), W(XG), W(XS))

#define mulcx_ld(XG, MS, DS)                                                \
        mulcx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulcx3rr(XD, XS, XT)                                                \
        EMITW(0x78400012 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78400012 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mulcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78400012 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78400012 | MXM(RYG(XD), RYG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #T) */

#undef  divcx_rr
#undef  divcx_ld
#undef  divcx3rr
#undef  divcx3ld

#define divcx_rr(XG, XS)                                                    \
        divcx3rr(W(XG), W(XG), W(XS))

#define divcx_ld(XG, MS, DS)                                                \
        divcx3ld(W(XG), W(XG), W(MS), W(DS))

#define divcx3rr(XD, XS, XT)                                                \
        EMITW(0x7AC00012 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7AC00012 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC00012 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC00012 | MXM(RYG(XD), RYG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #T) */

#undef  divcn_rr
#undef  divcn_ld
#undef  divcn3rr
#undef  divcn3ld

#define divcn_rr(XG, XS)                                                    \
        divcn3rr(W(XG), W(XG), W(XS))

#define divcn_ld(XG, MS, DS)                                                \
        divcn3ld(W(XG), W(XG), W(MS), W(DS))

#define divcn3rr(XD, XS, XT)                                                \
        EMITW(0x7A400012 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7A400012 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define divcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A400012 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A400012 | MXM(RYG(XD), RYG(XS), TmmM))

/* rem (G = G % S), (D = S % T) if (#D != #T) */

#undef  remcx_rr
#undef  remcx_ld
#undef  remcx3rr
#undef  remcx3ld

#define remcx_rr(XG, XS)                                                    \
        remcx3rr(W(XG), W(XG), W(XS))

#define remcx_ld(XG, MS, DS)                                                \
        remcx3ld(W(XG), W(XG), W(MS), W(DS))

#define remcx3rr(XD, XS, XT)                                                \
        EMITW(0x7BC00012 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7BC00012 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define remcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7BC00012 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7BC00012 | MXM(RYG(XD), RYG(XS), TmmM))

/* rem (G = G % S), (D = S % T) if (#D != #T) */

#undef  remcn_rr
#undef  remcn_ld
#undef  remcn3rr
#undef  remcn3ld

#define remcn_rr(XG, XS)                                                    \
        remcn3rr(W(XG), W(XG), W(XS))

#define remcn_ld(XG, MS, DS)                                                \
        remcn3ld(W(XG), W(XG), W(MS), W(DS))

#define remcn3rr(XD, XS, XT)                                                \
        EMITW(0x7B400012 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7B400012 | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define remcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B400012 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B400012 | MXM(RYG(XD), RYG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlcx_ri(XG, IS)                                                    \
        shlcx3ri(W(XG), W(XG), W(IS))

#define shlcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlcx3ri(XD, XS, IT)                                                \
        EMITW(0x78400009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)    \
        EMITW(0x78400009 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shlcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7840000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7840000D | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcx_ri(XG, IS)                                                    \
        shrcx3ri(W(XG), W(XG), W(IS))

#define shrcx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcx3ri(XD, XS, IT)                                                \
        EMITW(0x79400009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)    \
        EMITW(0x79400009 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7940000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x7940000D | MXM(RYG(XD), RYG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrcn_ri(XG, IS)                                                    \
        shrcn3ri(W(XG), W(XG), W(IS))

#define shrcn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrcn3ri(XD, XS, IT)                                                \
        EMITW(0x78C00009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)    \
        EMITW(0x78C00009 | MXM(RYG(XD), RYG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shrcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x78C0000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78C0000D | MXM(RYG(XD), RYG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlcx3rr(W(XG), W(XG), W(XS))

#define svlcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlcx3rr(XD, XS, XT)                                                \
        EMITW(0x7840000D | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7840000D | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svlcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000D | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcx3rr(W(XG), W(XG), W(XS))

#define svrcx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcx3rr(XD, XS, XT)                                                \
        EMITW(0x7940000D | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7940000D | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svrcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000D | MXM(RYG(XD), RYG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrcn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrcn3rr(W(XG), W(XG), W(XS))

#define svrcn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrcn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrcn3rr(XD, XS, XT)                                                \
        EMITW(0x78C0000D | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x78C0000D | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define svrcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0000D | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0000D | MXM(RYG(XD), RYG(XS), TmmM))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mincx_rr(XG, XS)                                                    \
        mincx3rr(W(XG), W(XG), W(XS))

#define mincx_ld(XG, MS, DS)                                                \
        mincx3ld(W(XG), W(XG), W(MS), W(DS))

#define mincx3rr(XD, XS, XT)                                                \
        EMITW(0x7AC0000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7AC0000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mincx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC0000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC0000E | MXM(RYG(XD), RYG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mincn_rr(XG, XS)                                                    \
        mincn3rr(W(XG), W(XG), W(XS))

#define mincn_ld(XG, MS, DS)                                                \
        mincn3ld(W(XG), W(XG), W(MS), W(DS))

#define mincn3rr(XD, XS, XT)                                                \
        EMITW(0x7A40000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7A40000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define mincn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A40000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A40000E | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxcx_rr(XG, XS)                                                    \
        maxcx3rr(W(XG), W(XG), W(XS))

#define maxcx_ld(XG, MS, DS)                                                \
        maxcx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcx3rr(XD, XS, XT)                                                \
        EMITW(0x79C0000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x79C0000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x79C0000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x79C0000E | MXM(RYG(XD), RYG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxcn_rr(XG, XS)                                                    \
        maxcn3rr(W(XG), W(XG), W(XS))

#define maxcn_ld(XG, MS, DS)                                                \
        maxcn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxcn3rr(XD, XS, XT)                                                \
        EMITW(0x7940000E | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7940000E | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define maxcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000E | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000E | MXM(RYG(XD), RYG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqcx_rr(XG, XS)                                                    \
        ceqcx3rr(W(XG), W(XG), W(XS))

#define ceqcx_ld(XG, MS, DS)                                                \
        ceqcx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqcx3rr(XD, XS, XT)                                                \
        EMITW(0x7840000F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7840000F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define ceqcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000F | MXM(RYG(XD), RYG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnecx_rr(XG, XS)                                                    \
        cnecx3rr(W(XG), W(XG), W(XS))

#define cnecx_ld(XG, MS, DS)                                                \
        cnecx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnecx3rr(XD, XS, XT)                                                \
        ceqcx3rr(W(XD), W(XS), W(XT))                                       \
        notcx_rx(W(XD))

#define cnecx3ld(XD, XS, MT, DT)                                            \
        ceqcx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notcx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltcx_rr(XG, XS)                                                    \
        cltcx3rr(W(XG), W(XG), W(XS))

#define cltcx_ld(XG, MS, DS)                                                \
        cltcx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcx3rr(XD, XS, XT)                                                \
        EMITW(0x79C0000F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x79C0000F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cltcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x79C0000F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x79C0000F | MXM(RYG(XD), RYG(XS), TmmM))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltcn_rr(XG, XS)                                                    \
        cltcn3rr(W(XG), W(XG), W(XS))

#define cltcn_ld(XG, MS, DS)                                                \
        cltcn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltcn3rr(XD, XS, XT)                                                \
        EMITW(0x7940000F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7940000F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define cltcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000F | MXM(RYG(XD), RYG(XS), TmmM))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clecx_rr(XG, XS)                                                    \
        clecx3rr(W(XG), W(XG), W(XS))

#define clecx_ld(XG, MS, DS)                                                \
        clecx3ld(W(XG), W(XG), W(MS), W(DS))

#define clecx3rr(XD, XS, XT)                                                \
        EMITW(0x7AC0000F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7AC0000F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define clecx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC0000F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC0000F | MXM(RYG(XD), RYG(XS), TmmM))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clecn_rr(XG, XS)                                                    \
        clecn3rr(W(XG), W(XG), W(XS))

#define clecn_ld(XG, MS, DS)                                                \
        clecn3ld(W(XG), W(XG), W(MS), W(DS))

#define clecn3rr(XD, XS, XT)                                                \
        EMITW(0x7A40000F | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x7A40000F | MXM(RYG(XD), RYG(XS), RYG(XT)))

#define clecn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A40000F | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A40000F | MXM(RYG(XD), RYG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtcx_rr(XG, XS)                                                    \
        cgtcx3rr(W(XG), W(XG), W(XS))

#define cgtcx_ld(XG, MS, DS)                                                \
        cgtcx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcx3rr(XD, XS, XT)                                                \
        EMITW(0x79C0000F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x79C0000F | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cgtcx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x79C0000F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x79C0000F | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtcn_rr(XG, XS)                                                    \
        cgtcn3rr(W(XG), W(XG), W(XS))

#define cgtcn_ld(XG, MS, DS)                                                \
        cgtcn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtcn3rr(XD, XS, XT)                                                \
        EMITW(0x7940000F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x7940000F | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cgtcn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000F | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgecx_rr(XG, XS)                                                    \
        cgecx3rr(W(XG), W(XG), W(XS))

#define cgecx_ld(XG, MS, DS)                                                \
        cgecx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecx3rr(XD, XS, XT)                                                \
        EMITW(0x7AC0000F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x7AC0000F | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cgecx3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC0000F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7AC0000F | MXM(RYG(XD), TmmM,    RYG(XS)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgecn_rr(XG, XS)                                                    \
        cgecn3rr(W(XG), W(XG), W(XS))

#define cgecn_ld(XG, MS, DS)                                                \
        cgecn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgecn3rr(XD, XS, XT)                                                \
        EMITW(0x7A40000F | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x7A40000F | MXM(RYG(XD), RYG(XT), RYG(XS)))

#define cgecn3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), K2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A40000F | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VYL(DT), B4(DT), K2(DT)))  \
    SJF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7A40000F | MXM(RYG(XD), TmmM,    RYG(XS)))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#undef  sregs_sa
#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movcx_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_st(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        EMITW(0x78000027 | MXM(TmmZ,    TEax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000027 | MXM(TmmM,    TEax,    0x00))

#undef  sregs_la
#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movcx_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        movcx_ld(XmmE, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_256*4))                           \
        EMITW(0x78000023 | MXM(TmmZ,    TEax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000023 | MXM(TmmM,    TEax,    0x00))

#endif /* RT_128X2 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M32_128X2V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

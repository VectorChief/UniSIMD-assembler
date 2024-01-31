/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_128X1V8_H
#define RT_RTARCH_X86_128X1V8_H

#include "rtarch_x86.h"

#define RT_SIMD_REGS_128        8

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x86_128x1v8.h: Implementation of x86 fp32 AVX1/2 instructions.
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

#if (RT_128X1 >= 8 && RT_128X1 <= 32)

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x03, EMPTY
#define Xmm1    0x01, 0x03, EMPTY
#define Xmm2    0x02, 0x03, EMPTY
#define Xmm3    0x03, 0x03, EMPTY
#define Xmm4    0x04, 0x03, EMPTY
#define Xmm5    0x05, 0x03, EMPTY
#define Xmm6    0x06, 0x03, EMPTY
#define Xmm7    0x07, 0x03, EMPTY

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmix_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        movrs_st(W(XS), W(MD), W(DD))

/***********   packed single/double-precision generic move/logic   ************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        V2X(0x00,    0, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movix_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movix_st(XS, MD, DD)                                                \
        V2X(0x00,    0, 0) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define movjx_rr(XD, XS)                                                    \
        V2X(0x00,    0, 1) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movjx_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 1) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movjx_st(XS, MD, DD)                                                \
        V2X(0x00,    0, 1) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvix_rr(XG, XS)                                                    \
        VEX(REG(XG), 0, 1, 3) EMITB(0x4A)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define mmvix_ld(XG, MS, DS)                                                \
        VEX(REG(XG), 0, 1, 3) EMITB(0x4A)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define mmvix_st(XS, MG, DG)                                                \
        VEX(0x00,    0, 1, 2) EMITB(0x2E)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)


#define mmvjx_rr(XG, XS)                                                    \
        VEX(REG(XG), 0, 1, 3) EMITB(0x4B)                                   \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define mmvjx_ld(XG, MS, DS)                                                \
        VEX(REG(XG), 0, 1, 3) EMITB(0x4B)                                   \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define mmvjx_st(XS, MG, DG)                                                \
        VEX(0x00,    0, 1, 2) EMITB(0x2F)                                   \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andix_rr(XG, XS)                                                    \
        andix3rr(W(XG), W(XG), W(XS))

#define andix_ld(XG, MS, DS)                                                \
        andix3ld(W(XG), W(XG), W(MS), W(DS))

#define andix3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define andjx_rr(XG, XS)                                                    \
        andjx3rr(W(XG), W(XG), W(XS))

#define andjx_ld(XG, MS, DS)                                                \
        andjx3ld(W(XG), W(XG), W(MS), W(DS))

#define andjx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andjx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x54)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annix_rr(XG, XS)                                                    \
        annix3rr(W(XG), W(XG), W(XS))

#define annix_ld(XG, MS, DS)                                                \
        annix3ld(W(XG), W(XG), W(MS), W(DS))

#define annix3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define annjx_rr(XG, XS)                                                    \
        annjx3rr(W(XG), W(XG), W(XS))

#define annjx_ld(XG, MS, DS)                                                \
        annjx3ld(W(XG), W(XG), W(MS), W(DS))

#define annjx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define annjx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x55)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrix_rr(XG, XS)                                                    \
        orrix3rr(W(XG), W(XG), W(XS))

#define orrix_ld(XG, MS, DS)                                                \
        orrix3ld(W(XG), W(XG), W(MS), W(DS))

#define orrix3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define orrjx_rr(XG, XS)                                                    \
        orrjx3rr(W(XG), W(XG), W(XS))

#define orrjx_ld(XG, MS, DS)                                                \
        orrjx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrjx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrjx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x56)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define ornix_rr(XG, XS)                                                    \
        notix_rx(W(XG))                                                     \
        orrix_rr(W(XG), W(XS))

#define ornix_ld(XG, MS, DS)                                                \
        notix_rx(W(XG))                                                     \
        orrix_ld(W(XG), W(MS), W(DS))

#define ornix3rr(XD, XS, XT)                                                \
        notix_rr(W(XD), W(XS))                                              \
        orrix_rr(W(XD), W(XT))

#define ornix3ld(XD, XS, MT, DT)                                            \
        notix_rr(W(XD), W(XS))                                              \
        orrix_ld(W(XD), W(MT), W(DT))


#define ornjx_rr(XG, XS)                                                    \
        notjx_rx(W(XG))                                                     \
        orrjx_rr(W(XG), W(XS))

#define ornjx_ld(XG, MS, DS)                                                \
        notjx_rx(W(XG))                                                     \
        orrjx_ld(W(XG), W(MS), W(DS))

#define ornjx3rr(XD, XS, XT)                                                \
        notjx_rr(W(XD), W(XS))                                              \
        orrjx_rr(W(XD), W(XT))

#define ornjx3ld(XD, XS, MT, DT)                                            \
        notjx_rr(W(XD), W(XS))                                              \
        orrjx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorix_rr(XG, XS)                                                    \
        xorix3rr(W(XG), W(XG), W(XS))

#define xorix_ld(XG, MS, DS)                                                \
        xorix3ld(W(XG), W(XG), W(MS), W(DS))

#define xorix3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define xorjx_rr(XG, XS)                                                    \
        xorjx3rr(W(XG), W(XG), W(XS))

#define xorjx_ld(XG, MS, DS)                                                \
        xorjx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorjx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorjx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x57)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notix_rx(XG)                                                        \
        notix_rr(W(XG), W(XG))

#define notix_rr(XD, XS)                                                    \
        annix3ld(W(XD), W(XS), Mebp, inf_GPC07)


#define notjx_rx(XG)                                                        \
        notjx_rr(W(XG), W(XG))

#define notjx_rr(XD, XS)                                                    \
        annjx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/********   packed single/double-precision floating-point arithmetic   ********/

/* neg (G = -G), (D = -S) */

#define negis_rx(XG)                                                        \
        negis_rr(W(XG), W(XG))

#define negis_rr(XD, XS)                                                    \
        xorix3ld(W(XD), W(XS), Mebp, inf_GPC06_32)


#define negjs_rx(XG)                                                        \
        negjs_rr(W(XG), W(XG))

#define negjs_rr(XD, XS)                                                    \
        xorjx3ld(W(XD), W(XS), Mebp, inf_GPC06_64)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addis_rr(XG, XS)                                                    \
        addis3rr(W(XG), W(XG), W(XS))

#define addis_ld(XG, MS, DS)                                                \
        addis3ld(W(XG), W(XG), W(MS), W(DS))

#define addis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define addjs_rr(XG, XS)                                                    \
        addjs3rr(W(XG), W(XG), W(XS))

#define addjs_ld(XG, MS, DS)                                                \
        addjs3ld(W(XG), W(XG), W(MS), W(DS))

#define addjs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addjs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  adpis3rr
#define adpis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 3) EMITB(0x7C)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#undef  adpis3ld
#define adpis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 3) EMITB(0x7C)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#undef  adpjs3rr
#define adpjs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x7C)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#undef  adpjs3ld
#define adpjs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x7C)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subis_rr(XG, XS)                                                    \
        subis3rr(W(XG), W(XG), W(XS))

#define subis_ld(XG, MS, DS)                                                \
        subis3ld(W(XG), W(XG), W(MS), W(DS))

#define subis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define subjs_rr(XG, XS)                                                    \
        subjs3rr(W(XG), W(XG), W(XS))

#define subjs_ld(XG, MS, DS)                                                \
        subjs3ld(W(XG), W(XG), W(MS), W(DS))

#define subjs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subjs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulis_rr(XG, XS)                                                    \
        mulis3rr(W(XG), W(XG), W(XS))

#define mulis_ld(XG, MS, DS)                                                \
        mulis3ld(W(XG), W(XG), W(MS), W(DS))

#define mulis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define muljs_rr(XG, XS)                                                    \
        muljs3rr(W(XG), W(XG), W(XS))

#define muljs_ld(XG, MS, DS)                                                \
        muljs3ld(W(XG), W(XG), W(MS), W(DS))

#define muljs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define muljs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divis_rr(XG, XS)                                                    \
        divis3rr(W(XG), W(XG), W(XS))

#define divis_ld(XG, MS, DS)                                                \
        divis3ld(W(XG), W(XG), W(MS), W(DS))

#define divis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)


#define divjs_rr(XG, XS)                                                    \
        divjs3rr(W(XG), W(XG), W(XS))

#define divjs_ld(XG, MS, DS)                                                \
        divjs3ld(W(XG), W(XG), W(MS), W(DS))

#define divjs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divjs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqris_rr(XD, XS)                                                    \
        V2X(0x00,    0, 0) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqris_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 0) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define sqrjs_rr(XD, XS)                                                    \
        V2X(0x00,    0, 1) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrjs_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 1) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        V2X(0x00,    0, 0) EMITB(0x53)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsis_rr(XG, XS) /* destroys XS */                                  \
        mulis_rr(W(XS), W(XG))                                              \
        mulis_rr(W(XS), W(XG))                                              \
        addis_rr(W(XG), W(XG))                                              \
        subis_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        V2X(0x00,    0, 0) EMITB(0x52)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssis_rr(XG, XS) /* destroys XS */                                  \
        mulis_rr(W(XS), W(XG))                                              \
        mulis_rr(W(XS), W(XG))                                              \
        subis_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulis_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulis_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_128X1 < 16)

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmais_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_rr(W(XS), W(XT))                                              \
        addis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmais_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_ld(W(XS), W(MT), W(DT))                                       \
        addis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmais_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmais_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_ld(W(XS), W(MT), W(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsis_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_rr(W(XS), W(XT))                                              \
        subis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulis_ld(W(XS), W(MT), W(DT))                                       \
        subis_rr(W(XG), W(XS))                                              \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsis_rr(XG, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        cvycs_ld(W(XS), W(MT), W(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movix_ld(W(XS), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_128X1 >= 16, FMA3 or AVX2 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmais_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0xB8)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0xBC)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_128X1 >= 16, FMA3 or AVX2 */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minis_rr(XG, XS)                                                    \
        minis3rr(W(XG), W(XG), W(XS))

#define minis_ld(XG, MS, DS)                                                \
        minis3ld(W(XG), W(XG), W(MS), W(DS))

#define minis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxis_rr(XG, XS)                                                    \
        maxis3rr(W(XG), W(XG), W(XS))

#define maxis_ld(XG, MS, DS)                                                \
        maxis3ld(W(XG), W(XG), W(MS), W(DS))

#define maxis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqis_rr(XG, XS)                                                    \
        ceqis3rr(W(XG), W(XG), W(XS))

#define ceqis_ld(XG, MS, DS)                                                \
        ceqis3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneis_rr(XG, XS)                                                    \
        cneis3rr(W(XG), W(XG), W(XS))

#define cneis_ld(XG, MS, DS)                                                \
        cneis3ld(W(XG), W(XG), W(MS), W(DS))

#define cneis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltis_rr(XG, XS)                                                    \
        cltis3rr(W(XG), W(XG), W(XS))

#define cltis_ld(XG, MS, DS)                                                \
        cltis3ld(W(XG), W(XG), W(MS), W(DS))

#define cltis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleis_rr(XG, XS)                                                    \
        cleis3rr(W(XG), W(XG), W(XS))

#define cleis_ld(XG, MS, DS)                                                \
        cleis3ld(W(XG), W(XG), W(MS), W(DS))

#define cleis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtis_rr(XG, XS)                                                    \
        cgtis3rr(W(XG), W(XG), W(XS))

#define cgtis_ld(XG, MS, DS)                                                \
        cgtis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeis_rr(XG, XS)                                                    \
        cgeis3rr(W(XG), W(XG), W(XS))

#define cgeis_ld(XG, MS, DS)                                                \
        cgeis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeis3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 0) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128    0x0F     /*  all satisfy the condition */

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        V2X(0x00,    0, 0) EMITB(0x50)                                      \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_128))                     \
        jeqxx_lb(lb)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        V2X(0x00,    0, 2) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        V2X(0x00,    0, 2) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        rnpis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        rnmis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        cvtis_rr(W(XD), W(XS))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        cvtis_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        cvtin_rr(W(XD), W(XS))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        cvtin_ld(W(XD), W(MS), W(DS))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndis_ld(XD, MS, DS)                                                \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtis_rr(XD, XS)                                                    \
        V2X(0x00,    0, 1) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtis_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 1) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtin_rr(XD, XS)                                                    \
        V2X(0x00,    0, 0) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtin_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 0) EMITB(0x5B)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        VEX(0x00,    0, 1, 3) EMITB(0x08)                                   \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvris_rr(XD, XS, mode)                                              \
        rnris_rr(W(XD), W(XS), mode)                                        \
        cvzis_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addix_rr(XG, XS)                                                    \
        addix3rr(W(XG), W(XG), W(XS))

#define addix_ld(XG, MS, DS)                                                \
        addix3ld(W(XG), W(XG), W(MS), W(DS))

#define addix3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xFE)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xFE)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subix_rr(XG, XS)                                                    \
        subix3rr(W(XG), W(XG), W(XS))

#define subix_ld(XG, MS, DS)                                                \
        subix3ld(W(XG), W(XG), W(MS), W(DS))

#define subix3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xFA)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xFA)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulix_rr(XG, XS)                                                    \
        mulix3rr(W(XG), W(XG), W(XS))

#define mulix_ld(XG, MS, DS)                                                \
        mulix3ld(W(XG), W(XG), W(MS), W(DS))

#define mulix3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x40)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulix3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x40)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlix_ri(XG, IS)                                                    \
        shlix3ri(W(XG), W(XG), W(IS))

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix3ld(W(XG), W(XG), W(MS), W(DS))

#define shlix3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 0, 1) EMITB(0x72)                                      \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xF2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrix_ri(XG, IS)                                                    \
        shrix3ri(W(XG), W(XG), W(IS))

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix3ld(W(XG), W(XG), W(MS), W(DS))

#define shrix3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 0, 1) EMITB(0x72)                                      \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrix3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xD2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrin_ri(XG, IS)                                                    \
        shrin3ri(W(XG), W(XG), W(IS))

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin3ld(W(XG), W(XG), W(MS), W(DS))

#define shrin3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 0, 1) EMITB(0x72)                                      \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrin3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xE2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#if (RT_128X1 < 32)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix3rr(W(XG), W(XG), W(XS))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix3ld(W(XG), W(XG), W(MS), W(DS))

#define svlix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlix_rx(W(XD))

#define svlix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlix_rx(W(XD))

#define svlix_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix3rr(W(XG), W(XG), W(XS))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix3ld(W(XG), W(XG), W(MS), W(DS))

#define svrix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrix_rx(W(XD))

#define svrix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrix_rx(W(XD))

#define svrix_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin3rr(W(XG), W(XG), W(XS))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin3ld(W(XG), W(XG), W(MS), W(DS))

#define svrin3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrin_rx(W(XD))

#define svrin3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrin_rx(W(XD))

#define svrin_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_128X1 >= 32, AVX2 */

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix3rr(W(XG), W(XG), W(XS))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix3ld(W(XG), W(XG), W(MS), W(DS))

#define svlix3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svlix3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x47)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix3rr(W(XG), W(XG), W(XS))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix3ld(W(XG), W(XG), W(MS), W(DS))

#define svrix3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrix3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x45)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin3rr(W(XG), W(XG), W(XS))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin3ld(W(XG), W(XG), W(MS), W(DS))

#define svrin3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define svrin3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x46)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_128X1 >= 32, AVX2 */

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minix_rr(XG, XS)                                                    \
        minix3rr(W(XG), W(XG), W(XS))

#define minix_ld(XG, MS, DS)                                                \
        minix3ld(W(XG), W(XG), W(MS), W(DS))

#define minix3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3B)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minix3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3B)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minin_rr(XG, XS)                                                    \
        minin3rr(W(XG), W(XG), W(XS))

#define minin_ld(XG, MS, DS)                                                \
        minin3ld(W(XG), W(XG), W(MS), W(DS))

#define minin3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x39)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minin3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x39)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxix_rr(XG, XS)                                                    \
        maxix3rr(W(XG), W(XG), W(XS))

#define maxix_ld(XG, MS, DS)                                                \
        maxix3ld(W(XG), W(XG), W(MS), W(DS))

#define maxix3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3F)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxix3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3F)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxin_rr(XG, XS)                                                    \
        maxin3rr(W(XG), W(XG), W(XS))

#define maxin_ld(XG, MS, DS)                                                \
        maxin3ld(W(XG), W(XG), W(MS), W(DS))

#define maxin3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3D)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxin3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3D)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqix_rr(XG, XS)                                                    \
        ceqix3rr(W(XG), W(XG), W(XS))

#define ceqix_ld(XG, MS, DS)                                                \
        ceqix3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqix3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0x76)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqix3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0x76)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneix_rr(XG, XS)                                                    \
        cneix3rr(W(XG), W(XG), W(XS))

#define cneix_ld(XG, MS, DS)                                                \
        cneix3ld(W(XG), W(XG), W(MS), W(DS))

#define cneix3rr(XD, XS, XT)                                                \
        ceqix3rr(W(XD), W(XS), W(XT))                                       \
        notix_rx(W(XD))

#define cneix3ld(XD, XS, MT, DT)                                            \
        ceqix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notix_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltix_rr(XG, XS)                                                    \
        cltix3rr(W(XG), W(XG), W(XS))

#define cltix_ld(XG, MS, DS)                                                \
        cltix3ld(W(XG), W(XG), W(MS), W(DS))

#define cltix3rr(XD, XS, XT)                                                \
        minix3rr(W(XD), W(XS), W(XT))                                       \
        cneix_rr(W(XD), W(XT))

#define cltix3ld(XD, XS, MT, DT)                                            \
        minix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneix_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltin_rr(XG, XS)                                                    \
        cltin3rr(W(XG), W(XG), W(XS))

#define cltin_ld(XG, MS, DS)                                                \
        cltin3ld(W(XG), W(XG), W(MS), W(DS))

#define cltin3rr(XD, XS, XT)                                                \
        cgtin3rr(W(XD), W(XT), W(XS))

#define cltin3ld(XD, XS, MT, DT)                                            \
        minin3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneix_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleix_rr(XG, XS)                                                    \
        cleix3rr(W(XG), W(XG), W(XS))

#define cleix_ld(XG, MS, DS)                                                \
        cleix3ld(W(XG), W(XG), W(MS), W(DS))

#define cleix3rr(XD, XS, XT)                                                \
        maxix3rr(W(XD), W(XS), W(XT))                                       \
        ceqix_rr(W(XD), W(XT))

#define cleix3ld(XD, XS, MT, DT)                                            \
        maxix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqix_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clein_rr(XG, XS)                                                    \
        clein3rr(W(XG), W(XG), W(XS))

#define clein_ld(XG, MS, DS)                                                \
        clein3ld(W(XG), W(XG), W(MS), W(DS))

#define clein3rr(XD, XS, XT)                                                \
        cgtin3rr(W(XD), W(XS), W(XT))                                       \
        notix_rx(W(XD))

#define clein3ld(XD, XS, MT, DT)                                            \
        cgtin3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notix_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtix_rr(XG, XS)                                                    \
        cgtix3rr(W(XG), W(XG), W(XS))

#define cgtix_ld(XG, MS, DS)                                                \
        cgtix3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtix3rr(XD, XS, XT)                                                \
        maxix3rr(W(XD), W(XS), W(XT))                                       \
        cneix_rr(W(XD), W(XT))

#define cgtix3ld(XD, XS, MT, DT)                                            \
        maxix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cneix_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtin_rr(XG, XS)                                                    \
        cgtin3rr(W(XG), W(XG), W(XS))

#define cgtin_ld(XG, MS, DS)                                                \
        cgtin3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtin3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0x66)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtin3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0x66)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeix_rr(XG, XS)                                                    \
        cgeix3rr(W(XG), W(XG), W(XS))

#define cgeix_ld(XG, MS, DS)                                                \
        cgeix3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeix3rr(XD, XS, XT)                                                \
        minix3rr(W(XD), W(XS), W(XT))                                       \
        ceqix_rr(W(XD), W(XT))

#define cgeix3ld(XD, XS, MT, DT)                                            \
        minix3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqix_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgein_rr(XG, XS)                                                    \
        cgein3rr(W(XG), W(XG), W(XS))

#define cgein_ld(XG, MS, DS)                                                \
        cgein3ld(W(XG), W(XG), W(MS), W(DS))

#define cgein3rr(XD, XS, XT)                                                \
        minin3rr(W(XD), W(XS), W(XT))                                       \
        ceqix_rr(W(XD), W(XT))

#define cgein3ld(XD, XS, MT, DT)                                            \
        minin3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqix_ld(W(XD), W(MT), W(DT))

/****************   packed half-precision generic move/logic   ****************/

/* mov (D = S) */

#define movgx_rr(XD, XS)                                                    \
        V2X(0x00,    0, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movgx_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 0) EMITB(0x28)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movgx_st(XS, MD, DD)                                                \
        V2X(0x00,    0, 0) EMITB(0x29)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgx_rr(XG, XS)                                                    \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_rr(Xmm0, W(XG))                                               \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_rr(W(XG), Xmm0)

#define mmvgx_ld(XG, MS, DS)                                                \
        notgx_rx(Xmm0)                                                      \
        andgx_rr(W(XG), Xmm0)                                               \
        anngx_ld(Xmm0, W(MS), W(DS))                                        \
        orrgx_rr(W(XG), Xmm0)

#define mmvgx_st(XS, MG, DG)                                                \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_ld(Xmm0, W(MG), W(DG))                                        \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_st(Xmm0, W(MG), W(DG))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andgx_rr(XG, XS)                                                    \
        andgx3rr(W(XG), W(XG), W(XS))

#define andgx_ld(XG, MS, DS)                                                \
        andgx3ld(W(XG), W(XG), W(MS), W(DS))

#define andgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xDB)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define andgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xDB)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define anngx_rr(XG, XS)                                                    \
        anngx3rr(W(XG), W(XG), W(XS))

#define anngx_ld(XG, MS, DS)                                                \
        anngx3ld(W(XG), W(XG), W(MS), W(DS))

#define anngx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xDF)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define anngx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xDF)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrgx_rr(XG, XS)                                                    \
        orrgx3rr(W(XG), W(XG), W(XS))

#define orrgx_ld(XG, MS, DS)                                                \
        orrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define orrgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xEB)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define orrgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xEB)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* orn (G = ~G | S), (D = ~S | T) if (#D != #T) */

#define orngx_rr(XG, XS)                                                    \
        notgx_rx(W(XG))                                                     \
        orrgx_rr(W(XG), W(XS))

#define orngx_ld(XG, MS, DS)                                                \
        notgx_rx(W(XG))                                                     \
        orrgx_ld(W(XG), W(MS), W(DS))

#define orngx3rr(XD, XS, XT)                                                \
        notgx_rr(W(XD), W(XS))                                              \
        orrgx_rr(W(XD), W(XT))

#define orngx3ld(XD, XS, MT, DT)                                            \
        notgx_rr(W(XD), W(XS))                                              \
        orrgx_ld(W(XD), W(MT), W(DT))

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorgx_rr(XG, XS)                                                    \
        xorgx3rr(W(XG), W(XG), W(XS))

#define xorgx_ld(XG, MS, DS)                                                \
        xorgx3ld(W(XG), W(XG), W(MS), W(DS))

#define xorgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xEF)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define xorgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xEF)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* not (G = ~G), (D = ~S) */

#define notgx_rx(XG)                                                        \
        notgx_rr(W(XG), W(XG))

#define notgx_rr(XD, XS)                                                    \
        anngx3ld(W(XD), W(XS), Mebp, inf_GPC07)

/*************   packed half-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgx_rr(XG, XS)                                                    \
        addgx3rr(W(XG), W(XG), W(XS))

#define addgx_ld(XG, MS, DS)                                                \
        addgx3ld(W(XG), W(XG), W(MS), W(DS))

#define addgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xFD)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xFD)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgx_rr(XG, XS)                                                    \
        adsgx3rr(W(XG), W(XG), W(XS))

#define adsgx_ld(XG, MS, DS)                                                \
        adsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xDD)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xDD)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgn_rr(XG, XS)                                                    \
        adsgn3rr(W(XG), W(XG), W(XS))

#define adsgn_ld(XG, MS, DS)                                                \
        adsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgn3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xED)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgn3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xED)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgx_rr(XG, XS)                                                    \
        subgx3rr(W(XG), W(XG), W(XS))

#define subgx_ld(XG, MS, DS)                                                \
        subgx3ld(W(XG), W(XG), W(MS), W(DS))

#define subgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xF9)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xF9)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgx_rr(XG, XS)                                                    \
        sbsgx3rr(W(XG), W(XG), W(XS))

#define sbsgx_ld(XG, MS, DS)                                                \
        sbsgx3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xD9)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xD9)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgn_rr(XG, XS)                                                    \
        sbsgn3rr(W(XG), W(XG), W(XS))

#define sbsgn_ld(XG, MS, DS)                                                \
        sbsgn3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgn3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xE9)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgn3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xE9)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgx_rr(XG, XS)                                                    \
        mulgx3rr(W(XG), W(XG), W(XS))

#define mulgx_ld(XG, MS, DS)                                                \
        mulgx3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgx3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xD5)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xD5)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgx_ri(XG, IS)                                                    \
        shlgx3ri(W(XG), W(XG), W(IS))

#define shlgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgx3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 0, 1) EMITB(0x71)                                      \
        MRM(0x06,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shlgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xF1)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgx_ri(XG, IS)                                                    \
        shrgx3ri(W(XG), W(XG), W(IS))

#define shrgx_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgx3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 0, 1) EMITB(0x71)                                      \
        MRM(0x02,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrgx3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xD1)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgn_ri(XG, IS)                                                    \
        shrgn3ri(W(XG), W(XG), W(IS))

#define shrgn_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgn3ri(XD, XS, IT)                                                \
        V2X(REG(XD), 0, 1) EMITB(0x71)                                      \
        MRM(0x04,    MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define shrgn3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xE1)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgx3rr(W(XG), W(XG), W(XS))

#define svlgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgx_rx(W(XD))

#define svlgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlgx_rx(W(XD))

#define svlgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgx_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgx3rr(W(XG), W(XG), W(XS))

#define svrgx_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgx3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgx3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgx_rx(W(XD))

#define svrgx3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgx_rx(W(XD))

#define svrgx_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgn_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgn3rr(W(XG), W(XG), W(XS))

#define svrgn_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgn3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgn3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgn_rx(W(XD))

#define svrgn3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgn_rx(W(XD))

#define svrgn_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movhx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrhn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed half-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingx_rr(XG, XS)                                                    \
        mingx3rr(W(XG), W(XG), W(XS))

#define mingx_ld(XG, MS, DS)                                                \
        mingx3ld(W(XG), W(XG), W(MS), W(DS))

#define mingx3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3A)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingx3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3A)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingn_rr(XG, XS)                                                    \
        mingn3rr(W(XG), W(XG), W(XS))

#define mingn_ld(XG, MS, DS)                                                \
        mingn3ld(W(XG), W(XG), W(MS), W(DS))

#define mingn3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0xEA)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingn3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0xEA)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgx_rr(XG, XS)                                                    \
        maxgx3rr(W(XG), W(XG), W(XS))

#define maxgx_ld(XG, MS, DS)                                                \
        maxgx3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgx3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3E)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgx3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3E)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgn_rr(XG, XS)                                                    \
        maxgn3rr(W(XG), W(XG), W(XS))

#define maxgn_ld(XG, MS, DS)                                                \
        maxgn3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgn3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0xEE)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgn3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0xEE)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgx_rr(XG, XS)                                                    \
        ceqgx3rr(W(XG), W(XG), W(XS))

#define ceqgx_ld(XG, MS, DS)                                                \
        ceqgx3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgx3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0x75)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqgx3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0x75)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegx_rr(XG, XS)                                                    \
        cnegx3rr(W(XG), W(XG), W(XS))

#define cnegx_ld(XG, MS, DS)                                                \
        cnegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegx3rr(XD, XS, XT)                                                \
        ceqgx3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define cnegx3ld(XD, XS, MT, DT)                                            \
        ceqgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgx_rr(XG, XS)                                                    \
        cltgx3rr(W(XG), W(XG), W(XS))

#define cltgx_ld(XG, MS, DS)                                                \
        cltgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgx3rr(XD, XS, XT)                                                \
        mingx3rr(W(XD), W(XS), W(XT))                                       \
        cnegx_rr(W(XD), W(XT))

#define cltgx3ld(XD, XS, MT, DT)                                            \
        mingx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegx_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgn_rr(XG, XS)                                                    \
        cltgn3rr(W(XG), W(XG), W(XS))

#define cltgn_ld(XG, MS, DS)                                                \
        cltgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgn3rr(XD, XS, XT)                                                \
        cgtgn3rr(W(XD), W(XT), W(XS))

#define cltgn3ld(XD, XS, MT, DT)                                            \
        mingn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegx_rr(XG, XS)                                                    \
        clegx3rr(W(XG), W(XG), W(XS))

#define clegx_ld(XG, MS, DS)                                                \
        clegx3ld(W(XG), W(XG), W(MS), W(DS))

#define clegx3rr(XD, XS, XT)                                                \
        maxgx3rr(W(XD), W(XS), W(XT))                                       \
        ceqgx_rr(W(XD), W(XT))

#define clegx3ld(XD, XS, MT, DT)                                            \
        maxgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgx_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegn_rr(XG, XS)                                                    \
        clegn3rr(W(XG), W(XG), W(XS))

#define clegn_ld(XG, MS, DS)                                                \
        clegn3ld(W(XG), W(XG), W(MS), W(DS))

#define clegn3rr(XD, XS, XT)                                                \
        cgtgn3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define clegn3ld(XD, XS, MT, DT)                                            \
        cgtgn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgx_rr(XG, XS)                                                    \
        cgtgx3rr(W(XG), W(XG), W(XS))

#define cgtgx_ld(XG, MS, DS)                                                \
        cgtgx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgx3rr(XD, XS, XT)                                                \
        maxgx3rr(W(XD), W(XS), W(XT))                                       \
        cnegx_rr(W(XD), W(XT))

#define cgtgx3ld(XD, XS, MT, DT)                                            \
        maxgx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegx_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgn_rr(XG, XS)                                                    \
        cgtgn3rr(W(XG), W(XG), W(XS))

#define cgtgn_ld(XG, MS, DS)                                                \
        cgtgn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgn3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0x65)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtgn3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0x65)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegx_rr(XG, XS)                                                    \
        cgegx3rr(W(XG), W(XG), W(XS))

#define cgegx_ld(XG, MS, DS)                                                \
        cgegx3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegx3rr(XD, XS, XT)                                                \
        mingx3rr(W(XD), W(XS), W(XT))                                       \
        ceqgx_rr(W(XD), W(XT))

#define cgegx3ld(XD, XS, MT, DT)                                            \
        mingx3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgx_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegn_rr(XG, XS)                                                    \
        cgegn3rr(W(XG), W(XG), W(XS))

#define cgegn_ld(XG, MS, DS)                                                \
        cgegn3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegn3rr(XD, XS, XT)                                                \
        mingn3rr(W(XD), W(XS), W(XT))                                       \
        ceqgx_rr(W(XD), W(XT))

#define cgegn3ld(XD, XS, MT, DT)                                            \
        mingn3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgx_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE16_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL16_128    0x0F     /*  all satisfy the condition */

#define mkjgx_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Rebp)                                                      \
        V2X(0x00,    0, 0) EMITB(0x50)                                      \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        shlix_ri(W(XS), IB(16))                                             \
        V2X(0x00,    0, 0) EMITB(0x50)                                      \
        MRM(0x05,    MOD(XS), REG(XS))                                      \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##16_128 & 0x1) << 1)))  \
        MRM(0x00,       0x03, 0x05)                                         \
        stack_ld(Rebp)                                                      \
        movgx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##16_128))                     \
        jeqxx_lb(lb)

/****************   packed byte-precision generic move/logic   ****************/

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvgb_rr(XG, XS)                                                    \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_rr(Xmm0, W(XG))                                               \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_rr(W(XG), Xmm0)

#define mmvgb_ld(XG, MS, DS)                                                \
        notgx_rx(Xmm0)                                                      \
        andgx_rr(W(XG), Xmm0)                                               \
        anngx_ld(Xmm0, W(MS), W(DS))                                        \
        orrgx_rr(W(XG), Xmm0)

#define mmvgb_st(XS, MG, DG)                                                \
        andgx_rr(W(XS), Xmm0)                                               \
        anngx_ld(Xmm0, W(MG), W(DG))                                        \
        orrgx_rr(Xmm0, W(XS))                                               \
        movgx_st(Xmm0, W(MG), W(DG))

/* move/logic instructions are sizeless and provided in 16-bit subset above */

/*************   packed byte-precision integer arithmetic/shifts   ************/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addgb_rr(XG, XS)                                                    \
        addgb3rr(W(XG), W(XG), W(XS))

#define addgb_ld(XG, MS, DS)                                                \
        addgb3ld(W(XG), W(XG), W(MS), W(DS))

#define addgb3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xFC)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addgb3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xFC)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, unsigned */

#define adsgb_rr(XG, XS)                                                    \
        adsgb3rr(W(XG), W(XG), W(XS))

#define adsgb_ld(XG, MS, DS)                                                \
        adsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgb3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xDC)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgb3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xDC)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ads (G = G + S), (D = S + T) if (#D != #T) - saturate, signed */

#define adsgc_rr(XG, XS)                                                    \
        adsgc3rr(W(XG), W(XG), W(XS))

#define adsgc_ld(XG, MS, DS)                                                \
        adsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define adsgc3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xEC)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define adsgc3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xEC)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subgb_rr(XG, XS)                                                    \
        subgb3rr(W(XG), W(XG), W(XS))

#define subgb_ld(XG, MS, DS)                                                \
        subgb3ld(W(XG), W(XG), W(MS), W(DS))

#define subgb3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xF8)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subgb3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xF8)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, unsigned */

#define sbsgb_rr(XG, XS)                                                    \
        sbsgb3rr(W(XG), W(XG), W(XS))

#define sbsgb_ld(XG, MS, DS)                                                \
        sbsgb3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgb3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xD8)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgb3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xD8)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sbs (G = G - S), (D = S - T) if (#D != #T) - saturate, signed */

#define sbsgc_rr(XG, XS)                                                    \
        sbsgc3rr(W(XG), W(XG), W(XS))

#define sbsgc_ld(XG, MS, DS)                                                \
        sbsgc3ld(W(XG), W(XG), W(MS), W(DS))

#define sbsgc3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 1) EMITB(0xE8)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define sbsgc3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 1) EMITB(0xE8)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulgb_rr(XG, XS)                                                    \
        mulgb3rr(W(XG), W(XG), W(XS))

#define mulgb_ld(XG, MS, DS)                                                \
        mulgb3ld(W(XG), W(XG), W(MS), W(DS))

#define mulgb3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulgb_rx(W(XD))

#define mulgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulgb_rx(W(XD))

#define mulgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x00))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x00))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x01))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x01))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x02))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x02))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x03))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x03))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x04))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x04))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x05))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x05))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x06))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x06))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x07))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x07))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x08))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x08))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x09))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x09))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0A))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0A))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0B))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0B))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0C))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0C))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0D))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0D))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0E))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0E))                              \
        movbx_ld(Recx,  Mebp, inf_SCR01(0x0F))                              \
        mulbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        movbx_st(Recx,  Mebp, inf_SCR01(0x0F))                              \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlgb_ri(XG, IS)                                                    \
        shlgb3ri(W(XG), W(XG), W(IS))

#define shlgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shlgb3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shlgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shlgb_xx() /* not portable, do not use outside */                   \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgb_ri(XG, IS)                                                    \
        shrgb3ri(W(XG), W(XG), W(IS))

#define shrgb_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgb3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgb_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgb_xx() /* not portable, do not use outside */                   \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrgc_ri(XG, IS)                                                    \
        shrgc3ri(W(XG), W(XG), W(IS))

#define shrgc_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define shrgc3ri(XD, XS, IT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ri(Recx, W(IT))                                               \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgc3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx, W(MT), W(DT))                                        \
        shrgc_xx()                                                          \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

#define shrgc_xx() /* not portable, do not use outside */                   \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlgb3rr(W(XG), W(XG), W(XS))

#define svlgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svlgb3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svlgb_rx(W(XD))

#define svlgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svlgb_rx(W(XD))

#define svlgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shlbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgb_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgb3rr(W(XG), W(XG), W(XS))

#define svrgb_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgb3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgb3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgb_rx(W(XD))

#define svrgb3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgb_rx(W(XD))

#define svrgb_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbx_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrgc_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrgc3rr(W(XG), W(XG), W(XS))

#define svrgc_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrgc3ld(W(XG), W(XG), W(MS), W(DS))

#define svrgc3rr(XD, XS, XT)                                                \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_st(W(XT), Mebp, inf_SCR02(0))                                 \
        svrgc_rx(W(XD))

#define svrgc3ld(XD, XS, MT, DT)                                            \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movgx_ld(W(XD), W(MT), W(DT))                                       \
        movgx_st(W(XD), Mebp, inf_SCR02(0))                                 \
        svrgc_rx(W(XD))

#define svrgc_rx(XD) /* not portable, do not use outside */                 \
        stack_st(Recx)                                                      \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x01))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x01))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x02))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x02))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x03))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x03))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x05))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x05))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x06))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x06))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x07))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x07))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x09))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x09))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0A))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0A))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0B))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0B))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0D))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0D))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0E))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0E))                                    \
        movbx_ld(Recx,  Mebp, inf_SCR02(0x0F))                              \
        shrbn_mx(Mebp,  inf_SCR01(0x0F))                                    \
        stack_ld(Recx)                                                      \
        movgx_ld(W(XD), Mebp, inf_SCR01(0))

/*****************   packed byte-precision integer compare   ******************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define mingb_rr(XG, XS)                                                    \
        mingb3rr(W(XG), W(XG), W(XS))

#define mingb_ld(XG, MS, DS)                                                \
        mingb3ld(W(XG), W(XG), W(MS), W(DS))

#define mingb3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0xDA)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingb3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0xDA)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define mingc_rr(XG, XS)                                                    \
        mingc3rr(W(XG), W(XG), W(XS))

#define mingc_ld(XG, MS, DS)                                                \
        mingc3ld(W(XG), W(XG), W(MS), W(DS))

#define mingc3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x38)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mingc3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x38)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxgb_rr(XG, XS)                                                    \
        maxgb3rr(W(XG), W(XG), W(XS))

#define maxgb_ld(XG, MS, DS)                                                \
        maxgb3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgb3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0xDE)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgb3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0xDE)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxgc_rr(XG, XS)                                                    \
        maxgc3rr(W(XG), W(XG), W(XS))

#define maxgc_ld(XG, MS, DS)                                                \
        maxgc3ld(W(XG), W(XG), W(MS), W(DS))

#define maxgc3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3C)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxgc3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0x3C)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqgb_rr(XG, XS)                                                    \
        ceqgb3rr(W(XG), W(XG), W(XS))

#define ceqgb_ld(XG, MS, DS)                                                \
        ceqgb3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqgb3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0x74)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define ceqgb3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0x74)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cnegb_rr(XG, XS)                                                    \
        cnegb3rr(W(XG), W(XG), W(XS))

#define cnegb_ld(XG, MS, DS)                                                \
        cnegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cnegb3rr(XD, XS, XT)                                                \
        ceqgb3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define cnegb3ld(XD, XS, MT, DT)                                            \
        ceqgb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltgb_rr(XG, XS)                                                    \
        cltgb3rr(W(XG), W(XG), W(XS))

#define cltgb_ld(XG, MS, DS)                                                \
        cltgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgb3rr(XD, XS, XT)                                                \
        mingb3rr(W(XD), W(XS), W(XT))                                       \
        cnegb_rr(W(XD), W(XT))

#define cltgb3ld(XD, XS, MT, DT)                                            \
        mingb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegb_ld(W(XD), W(MT), W(DT))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltgc_rr(XG, XS)                                                    \
        cltgc3rr(W(XG), W(XG), W(XS))

#define cltgc_ld(XG, MS, DS)                                                \
        cltgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cltgc3rr(XD, XS, XT)                                                \
        cgtgc3rr(W(XD), W(XT), W(XS))

#define cltgc3ld(XD, XS, MT, DT)                                            \
        mingc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define clegb_rr(XG, XS)                                                    \
        clegb3rr(W(XG), W(XG), W(XS))

#define clegb_ld(XG, MS, DS)                                                \
        clegb3ld(W(XG), W(XG), W(MS), W(DS))

#define clegb3rr(XD, XS, XT)                                                \
        maxgb3rr(W(XD), W(XS), W(XT))                                       \
        ceqgb_rr(W(XD), W(XT))

#define clegb3ld(XD, XS, MT, DT)                                            \
        maxgb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgb_ld(W(XD), W(MT), W(DT))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clegc_rr(XG, XS)                                                    \
        clegc3rr(W(XG), W(XG), W(XS))

#define clegc_ld(XG, MS, DS)                                                \
        clegc3ld(W(XG), W(XG), W(MS), W(DS))

#define clegc3rr(XD, XS, XT)                                                \
        cgtgc3rr(W(XD), W(XS), W(XT))                                       \
        notgx_rx(W(XD))

#define clegc3ld(XD, XS, MT, DT)                                            \
        cgtgc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        notgx_rx(W(XD))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtgb_rr(XG, XS)                                                    \
        cgtgb3rr(W(XG), W(XG), W(XS))

#define cgtgb_ld(XG, MS, DS)                                                \
        cgtgb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgb3rr(XD, XS, XT)                                                \
        maxgb3rr(W(XD), W(XS), W(XT))                                       \
        cnegb_rr(W(XD), W(XT))

#define cgtgb3ld(XD, XS, MT, DT)                                            \
        maxgb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        cnegb_ld(W(XD), W(MT), W(DT))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtgc_rr(XG, XS)                                                    \
        cgtgc3rr(W(XG), W(XG), W(XS))

#define cgtgc_ld(XG, MS, DS)                                                \
        cgtgc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtgc3rr(XD, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 1) EMITB(0x64)                                   \
        MRM(REG(XD), MOD(XT), REG(XT))

#define cgtgc3ld(XD, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 1) EMITB(0x64)                                   \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgegb_rr(XG, XS)                                                    \
        cgegb3rr(W(XG), W(XG), W(XS))

#define cgegb_ld(XG, MS, DS)                                                \
        cgegb3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegb3rr(XD, XS, XT)                                                \
        mingb3rr(W(XD), W(XS), W(XT))                                       \
        ceqgb_rr(W(XD), W(XT))

#define cgegb3ld(XD, XS, MT, DT)                                            \
        mingb3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgb_ld(W(XD), W(MT), W(DT))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgegc_rr(XG, XS)                                                    \
        cgegc3rr(W(XG), W(XG), W(XS))

#define cgegc_ld(XG, MS, DS)                                                \
        cgegc3ld(W(XG), W(XG), W(MS), W(DS))

#define cgegc3rr(XD, XS, XT)                                                \
        mingc3rr(W(XD), W(XS), W(XT))                                       \
        ceqgb_rr(W(XD), W(XT))

#define cgegc3ld(XD, XS, MT, DT)                                            \
        mingc3ld(W(XD), W(XS), W(MT), W(DT))                                \
        ceqgb_ld(W(XD), W(MT), W(DT))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE08_128    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL08_128    0x0F     /*  all satisfy the condition */

#define bsnix_rx(XS, mask) /* not portable, do not use outside */           \
        V2X(0x00,    0, 0) EMITB(0x50)                                      \
        MRM(0x05,    MOD(XS), REG(XS))                                      \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##08_128 & 0x1) << 1)))  \
        MRM(0x00,       0x03, 0x05)

#define mkjgb_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        movgx_st(W(XS), Mebp, inf_SCR01(0))                                 \
        stack_st(Rebp)                                                      \
        V2X(0x00,    0, 0) EMITB(0x50)                                      \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        shlix_ri(W(XS), IB(8))                                              \
        bsnix_rx(W(XS), mask)                                               \
        shlix_ri(W(XS), IB(8))                                              \
        bsnix_rx(W(XS), mask)                                               \
        shlix_ri(W(XS), IB(8))                                              \
        bsnix_rx(W(XS), mask)                                               \
        stack_ld(Rebp)                                                      \
        movgx_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask##08_128))                     \
        jeqxx_lb(lb)

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*********   scalar single-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movrs_rr(XD, XS)                                                    \
        V2X(REG(XD), 0, 2) EMITB(0x10)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movrs_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 2) EMITB(0x10)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movrs_st(XS, MD, DD)                                                \
        V2X(0x00,    0, 2) EMITB(0x11)                                      \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addrs_rr(XG, XS)                                                    \
        addrs3rr(W(XG), W(XG), W(XS))

#define addrs_ld(XG, MS, DS)                                                \
        addrs3ld(W(XG), W(XG), W(MS), W(DS))

#define addrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0x58)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subrs_rr(XG, XS)                                                    \
        subrs3rr(W(XG), W(XG), W(XS))

#define subrs_ld(XG, MS, DS)                                                \
        subrs3ld(W(XG), W(XG), W(MS), W(DS))

#define subrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0x5C)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulrs_rr(XG, XS)                                                    \
        mulrs3rr(W(XG), W(XG), W(XS))

#define mulrs_ld(XG, MS, DS)                                                \
        mulrs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0x59)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divrs_rr(XG, XS)                                                    \
        divrs3rr(W(XG), W(XG), W(XS))

#define divrs_ld(XG, MS, DS)                                                \
        divrs3ld(W(XG), W(XG), W(MS), W(DS))

#define divrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0x5E)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
        V2X(0x00,    0, 2) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqrrs_ld(XD, MS, DS)                                                \
        V2X(0x00,    0, 2) EMITB(0x51)                                      \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
        V2X(0x00,    0, 2) EMITB(0x53)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsrs_rr(XG, XS) /* destroys XS */                                  \
        mulrs_rr(W(XS), W(XG))                                              \
        mulrs_rr(W(XS), W(XG))                                              \
        addrs_rr(W(XG), W(XG))                                              \
        subrs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
        V2X(0x00,    0, 2) EMITB(0x52)                                      \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssrs_rr(XG, XS) /* destroys XS */                                  \
        mulrs_rr(W(XS), W(XG))                                              \
        mulrs_rr(W(XS), W(XG))                                              \
        subrs_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulrs_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulrs_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_128X1 < 16)

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmars_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_rr(W(XS), W(XT))                                              \
        addrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#define fmars_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_ld(W(XS), W(MT), W(DT))                                       \
        addrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMR == 0

#define fmars_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmars_rx(W(XG))

#define fmars_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmars_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmars_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmars_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmars_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmars_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmars_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        movrs_st(W(XG), Mebp, inf_SCR02(0))                                 \
        addws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movrs_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsrs_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_rr(W(XS), W(XT))                                              \
        subrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulrs_ld(W(XS), W(MT), W(DT))                                       \
        subrs_rr(W(XG), W(XS))                                              \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMR == 0

#define fmsrs_rr(XG, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsrs_rx(W(XG))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsrs_rx(W(XG))

#elif RT_SIMD_COMPAT_FMR == 1

#define fmsrs_rr(XG, XS, XT)                                                \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        fmsrs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XS), W(MT), W(DT))                                       \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XS), Mebp, inf_SCR01(0))                                 \
        fmsrs_rx(W(XG))                                                     \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x037F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMR */

#define fmsrs_rx(XG) /* not portable, do not use outside */                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        mulws_ld(Mebp,  inf_SCR02(0x00))                                    \
        movrs_st(W(XG), Mebp, inf_SCR02(0))                                 \
        sbrws_ld(Mebp,  inf_SCR02(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR02(0x00))                                    \
        movrs_ld(W(XG), Mebp, inf_SCR02(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_128X1 >= 16, FMA3 or AVX2 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0xB9)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmars_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0xB9)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        VEX(REG(XS), 0, 1, 2) EMITB(0xBD)                                   \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        VEX(REG(XS), 0, 1, 2) EMITB(0xBD)                                   \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_128X1 >= 16, FMA3 or AVX2 */

/*************   scalar single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minrs_rr(XG, XS)                                                    \
        minrs3rr(W(XG), W(XG), W(XS))

#define minrs_ld(XG, MS, DS)                                                \
        minrs3ld(W(XG), W(XG), W(MS), W(DS))

#define minrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define minrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0x5D)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxrs_rr(XG, XS)                                                    \
        maxrs3rr(W(XG), W(XG), W(XS))

#define maxrs_ld(XG, MS, DS)                                                \
        maxrs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0x5F)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqrs_rr(XG, XS)                                                    \
        ceqrs3rr(W(XG), W(XG), W(XS))

#define ceqrs_ld(XG, MS, DS)                                                \
        ceqrs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x00))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cners_rr(XG, XS)                                                    \
        cners3rr(W(XG), W(XG), W(XS))

#define cners_ld(XG, MS, DS)                                                \
        cners3ld(W(XG), W(XG), W(MS), W(DS))

#define cners3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cners3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x04))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltrs_rr(XG, XS)                                                    \
        cltrs3rr(W(XG), W(XG), W(XS))

#define cltrs_ld(XG, MS, DS)                                                \
        cltrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x01))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clers_rr(XG, XS)                                                    \
        clers3rr(W(XG), W(XG), W(XS))

#define clers_ld(XG, MS, DS)                                                \
        clers3ld(W(XG), W(XG), W(MS), W(DS))

#define clers3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define clers3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x02))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtrs_rr(XG, XS)                                                    \
        cgtrs3rr(W(XG), W(XG), W(XS))

#define cgtrs_ld(XG, MS, DS)                                                \
        cgtrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtrs3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtrs3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x06))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgers_rr(XG, XS)                                                    \
        cgers3rr(W(XG), W(XG), W(XS))

#define cgers_ld(XG, MS, DS)                                                \
        cgers3ld(W(XG), W(XG), W(MS), W(DS))

#define cgers3rr(XD, XS, XT)                                                \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgers3ld(XD, XS, MT, DT)                                            \
        V2X(REG(XS), 0, 2) EMITB(0xC2)                                      \
        MRM(REG(XD), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMITB(0x05))

/******************************************************************************/
/**********************************   MODE   **********************************/
/******************************************************************************/

/************************   helper macros (FPU mode)   ************************/

/* simd mode

 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/POWER),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x01    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x07    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x05    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x07    /* round towards zero */

#define mxcsr_ld(MS, DS) /* not portable, do not use outside */             \
        V2X(0x00,    0, 0) EMITB(0xAE)                                      \
        MRM(0x02,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mxcsr_st(MD, DD) /* not portable, do not use outside */             \
        V2X(0x00,    0, 0) EMITB(0xAE)                                      \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        movwx_mi(Mebp, inf_SCR02(4), IH(RT_SIMD_MODE_##mode << 13 | 0x1F80))\
        mxcsr_ld(Mebp, inf_SCR02(4))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_ROUNDN&3)*4))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_##mode&3)*4))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        mxcsr_ld(Mebp, inf_FCTRL((RT_SIMD_MODE_ROUNDN&3)*4))

#endif /* RT_SIMD_FAST_FCTRL */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#ifndef RT_RTARCH_X86_256X1V2_H
#undef  RT_256X1
#define RT_256X1  (RT_128X1/16 + (RT_128X1 == 8))
#include "rtarch_x86_256x1v2.h"
#endif /* RT_RTARCH_X86_256X1V2_H */

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_128X1V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2025 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P32_128X1V4_H
#define RT_RTARCH_P32_128X1V4_H

#include "rtarch_p64.h"
#include "rtarch_pHB_128x1v4.h"

#define RT_SIMD_REGS_128        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p32_128x1v4.h: Implementation of POWER fp32 VMX instructions.
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

#if (RT_128X1 >= 4)

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 11 | (ren) << 16 | (reg) << 21)

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 16 | (reg) << 21)

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  B4(val, tp1, tp2)  B4##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2
#define  A2(val, tp1, tp2)  A2##tp2

#define  L1(val, tp1, tp2)  L1##tp1
#define  U1(val, tp1, tp2)  U1##tp1

/* displacement encoding SIMD(TP2), ELEM(TP2) */

#define B20(br) (br)
#define B40(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0x7FFC))
#define C20(br, dp) EMPTY
#define A20(br, dp) EMPTY

#define B21(br) (br)
#define B41(br) TPxx
#define P21(dp) (0x44000214 | TDxx << 11)
#define C21(br, dp) EMITW(0x60000000 | TDxx << 16 | (0xFFFC & (dp)))
#define A21(br, dp) C21(br, dp)                                             \
                    EMITW(0x7C000214 | MRM(TPxx,    (br),    TDxx))

#define B22(br) (br)
#define B42(br) TPxx
#define P22(dp) (0x44000214 | TDxx << 11)
#define C22(br, dp) EMITW(0x64000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x60000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFFC & (dp)))
#define A22(br, dp) C22(br, dp)                                             \
                    EMITW(0x7C000214 | MRM(TPxx,    (br),    TDxx))


#define L10(dp) (0xC0000000 |(0x7FFC & (dp)))
#define U10(dp) (0xD0000000 |(0x7FFC & (dp)))

#define L11(dp) (0x7C00042E | TDxx << 11)
#define U11(dp) (0x7C00052E | TDxx << 11)

#define L12(dp) (0x7C00042E | TDxx << 11)
#define U12(dp) (0x7C00052E | TDxx << 11)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tff1    0x11  /* f17 */
#define Tff2    0x12  /* f18 */

#define TmmR    0x18  /* v24, VMX only, Rounding Mode */
#define TmmS    0x19  /* v25, VMX only, sign-mask 32-bit */
#define TmmT    0x1E  /* v30, VMX only */
#define TmmU    0x1A  /* v26, VMX only, +1.0 32-bit */
#define TmmV    0x1B  /* v27, VMX only, -0.5 32-bit */
#define TmmW    0x1C  /* v28, VMX only */
#define TmmZ    0x1D  /* v29, VMX only */

#define TmmQ    0x0F  /* v15, internal name for all-ones, not persistent */
#define TmmM    0x1F  /* v31, temp-reg name for mem-args */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, 0x00, EMPTY       /* v0 */
#define Xmm1    0x01, 0x00, EMPTY       /* v1 */
#define Xmm2    0x02, 0x00, EMPTY       /* v2 */
#define Xmm3    0x03, 0x00, EMPTY       /* v3 */
#define Xmm4    0x04, 0x00, EMPTY       /* v4 */
#define Xmm5    0x05, 0x00, EMPTY       /* v5 */
#define Xmm6    0x06, 0x00, EMPTY       /* v6 */
#define Xmm7    0x07, 0x00, EMPTY       /* v7 */

/* only for 128-bit instructions (save/restore in 256-bit header)
 * provided as an extension to common baseline of 8 registers */

#define Xmm8    0x08, 0x00, EMPTY       /* v8 */
#define Xmm9    0x09, 0x00, EMPTY       /* v9 */
#define XmmA    0x0A, 0x00, EMPTY       /* v10 */
#define XmmB    0x0B, 0x00, EMPTY       /* v11 */
#define XmmC    0x0C, 0x00, EMPTY       /* v12 */
#define XmmD    0x0D, 0x00, EMPTY       /* v13 */
#define XmmE    0x0E, 0x00, EMPTY       /* v14 */
#define XmmF    0x10, 0x00, EMPTY       /* v16 */

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmix_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    REG(XS)))                  \
        EMITW(0x7C00018E | MXM(TmmM,    TEax & M(MOD(MD) == TPxx), TPxx))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), REG(XS)))

#define movix_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(REG(XD), TEax & M(MOD(MS) == TPxx), TPxx))

#define movix_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x7C0001CE | MXM(REG(XS), TEax & M(MOD(MD) == TPxx), TPxx))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, 0-masked XS elems */

#define mmvix_rr(XG, XS)                                                    \
        EMITW(0x1000002A | MXM(REG(XG), REG(XG), REG(XS)))

#define mmvix_ld(XG, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000002A | MXM(REG(XG), REG(XG), TmmM))

#define mmvix_st(XS, MG, DG)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MG) == TPxx), TPxx))   \
        EMITW(0x1000002A | MXM(TmmM,    TmmM,    REG(XS)))                  \
        EMITW(0x7C0001CE | MXM(TmmM,    TEax & M(MOD(MG) == TPxx), TPxx))

/* and (G = G & S), (D = S & T) if (#D != #T) */

#define andix_rr(XG, XS)                                                    \
        andix3rr(W(XG), W(XG), W(XS))

#define andix_ld(XG, MS, DS)                                                \
        andix3ld(W(XG), W(XG), W(MS), W(DS))

#define andix3rr(XD, XS, XT)                                                \
        EMITW(0x10000404 | MXM(REG(XD), REG(XS), REG(XT)))

#define andix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000404 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #T) */

#define annix_rr(XG, XS)                                                    \
        annix3rr(W(XG), W(XG), W(XS))

#define annix_ld(XG, MS, DS)                                                \
        annix3ld(W(XG), W(XG), W(MS), W(DS))

#define annix3rr(XD, XS, XT)                                                \
        EMITW(0x10000444 | MXM(REG(XD), REG(XT), REG(XS)))

#define annix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000444 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (#D != #T) */

#define orrix_rr(XG, XS)                                                    \
        orrix3rr(W(XG), W(XG), W(XS))

#define orrix_ld(XG, MS, DS)                                                \
        orrix3ld(W(XG), W(XG), W(MS), W(DS))

#define orrix3rr(XD, XS, XT)                                                \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), TmmM))

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #T) */

#define xorix_rr(XG, XS)                                                    \
        xorix3rr(W(XG), W(XG), W(XS))

#define xorix_ld(XG, MS, DS)                                                \
        xorix3ld(W(XG), W(XG), W(MS), W(DS))

#define xorix3rr(XD, XS, XT)                                                \
        EMITW(0x100004C4 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100004C4 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notix_rx(XG)                                                        \
        notix_rr(W(XG), W(XG))

#define notix_rr(XD, XS)                                                    \
        EMITW(0x10000504 | MXM(REG(XD), REG(XS), REG(XS)))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negis_rx(XG)                                                        \
        negis_rr(W(XG), W(XG))

#define negis_rr(XD, XS)                                                    \
        EMITW(0x100004C4 | MXM(REG(XD), REG(XS), TmmS))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addis_rr(XG, XS)                                                    \
        addis3rr(W(XG), W(XG), W(XS))

#define addis_ld(XG, MS, DS)                                                \
        addis3ld(W(XG), W(XG), W(MS), W(DS))

#define addis3rr(XD, XS, XT)                                                \
        EMITW(0x1000000A | MXM(REG(XD), REG(XS), REG(XT)))

#define addis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000000A | MXM(REG(XD), REG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subis_rr(XG, XS)                                                    \
        subis3rr(W(XG), W(XG), W(XS))

#define subis_ld(XG, MS, DS)                                                \
        subis3ld(W(XG), W(XG), W(MS), W(DS))

#define subis3rr(XD, XS, XT)                                                \
        EMITW(0x1000004A | MXM(REG(XD), REG(XS), REG(XT)))

#define subis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000004A | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulis_rr(XG, XS)                                                    \
        mulis3rr(W(XG), W(XG), W(XS))

#define mulis_ld(XG, MS, DS)                                                \
        mulis3ld(W(XG), W(XG), W(MS), W(DS))

#define mulis3rr(XD, XS, XT)                                                \
        EMITW(0x1000002E | MXM(REG(XD), REG(XS), TmmS) | REG(XT) << 6)

#define mulis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000002E | MXM(REG(XD), REG(XS), TmmS) | TmmM << 6)

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divis_rr(XG, XS)                                                    \
        divis3rr(W(XG), W(XG), W(XS))

#define divis_ld(XG, MS, DS)                                                \
        divis3ld(W(XG), W(XG), W(MS), W(DS))

/* internal definitions for IEEE-compatible div & sqr */

#define movws_ld(fd, MS, DS) /* not portable, do not use outside */         \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(fd,      MOD(MS), VAL(DS), B1(DS), L1(DS)))

#define movws_st(fs, MD, DD) /* not portable, do not use outside */         \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(fs,      MOD(MD), VAL(DD), B1(DD), U1(DD)))

#define divws_rr(fg, fs) /* not portable, do not use outside */             \
        EMITW(0xEC000024 | MTM(fg,      fg,      fs))

#define sqrws_rr(fd, fs) /* not portable, do not use outside */             \
        EMITW(0xEC00002C | MTM(fd,      0x00,    fs))

#if RT_SIMD_COMPAT_DIV == 1

#define divis3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x04))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x08))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x0C))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define divis3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x04))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x08))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x0C))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_DIV */

#define divis3rr(XD, XS, XT)                                                \
        EMITW(0x1000010A | MXM(TmmW,    0x00,    REG(XT)))                  \
        EMITW(0x1000002F | MXM(TmmZ,    TmmW,    TmmU) | REG(XT) << 6)      \
        EMITW(0x1000002E | MXM(TmmW,    TmmW,    TmmW) | TmmZ << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XS), TmmS) | TmmW << 6)         \
        EMITW(0x1000002F | MXM(REG(XD), TmmZ, REG(XS)) | REG(XT) << 6)      \
        EMITW(0x1000002E | MXM(REG(XD), REG(XD), TmmZ) | TmmW << 6)

#define divis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000010A | MXM(TmmW,    0x00,    TmmM))                     \
        EMITW(0x1000002F | MXM(TmmZ,    TmmW,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002E | MXM(TmmW,    TmmW,    TmmW) | TmmZ << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XS), TmmS) | TmmW << 6)         \
        EMITW(0x1000002F | MXM(REG(XD), TmmZ, REG(XS)) | TmmM << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), REG(XD), TmmZ) | TmmW << 6)

#endif /* RT_SIMD_COMPAT_DIV */

/* sqr (D = sqrt S) */

#if RT_SIMD_COMPAT_SQR == 1

#define sqris_rr(XD, XS)                                                    \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#define sqris_ld(XD, MS, DS)                                                \
        movix_ld(W(XD), W(MS), W(DS))                                       \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x04))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x04))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x08))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x08))                              \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x0C))                              \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_SQR */

#define sqris_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(TmmW,    0x00,    REG(XS)))                  \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmW,    TmmS) | REG(XS) << 6)

#define sqris_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000014A | MXM(TmmW,    0x00,    TmmM))                     \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmW,    TmmS) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_SQR */

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITW(0x1000010A | MXM(REG(XD), 0x00,    REG(XS)))

#define rcsis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1000002F | MXM(REG(XS), REG(XG), TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), REG(XG)) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(REG(XD), 0x00,    REG(XS)))

#define rssis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XG), TmmS) | REG(XG) << 6)      \
        EMITW(0x1000002E | MXM(TmmW,    REG(XG), TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(REG(XG), TmmZ,    REG(XG)) | TmmW << 6)

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minis_rr(XG, XS)                                                    \
        minis3rr(W(XG), W(XG), W(XS))

#define minis_ld(XG, MS, DS)                                                \
        minis3ld(W(XG), W(XG), W(MS), W(DS))

#define minis3rr(XD, XS, XT)                                                \
        EMITW(0x1000044A | MXM(REG(XD), REG(XS), REG(XT)))

#define minis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000044A | MXM(REG(XD), REG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  mnpis_rx
#define mnpis_rx(XD) /* not portable, do not use outside */                 \
        movrs2ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        minrs2ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs2ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        minrs2ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs2ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        minrs2ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs2ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        minrs2ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x0C))

#define movrs2ld(XD, MS, DS) /* not portable, do not use outside */         \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(REG(XD), TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(REG(XD), SPL(W(DS)), REG(XD)))

#define minrs2ld(XG, MS, DS) /* not portable, do not use outside */         \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DS)), TmmM))                     \
        EMITW(0x1000044A | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxis_rr(XG, XS)                                                    \
        maxis3rr(W(XG), W(XG), W(XS))

#define maxis_ld(XG, MS, DS)                                                \
        maxis3ld(W(XG), W(XG), W(MS), W(DS))

#define maxis3rr(XD, XS, XT)                                                \
        EMITW(0x1000040A | MXM(REG(XD), REG(XS), REG(XT)))

#define maxis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000040A | MXM(REG(XD), REG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#undef  mxpis_rx
#define mxpis_rx(XD) /* not portable, do not use outside */                 \
        movrs2ld(W(XD), Mebp, inf_SCR01(0x00))                              \
        maxrs2ld(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x00))                              \
        movrs2ld(W(XD), Mebp, inf_SCR01(0x08))                              \
        maxrs2ld(W(XD), Mebp, inf_SCR01(0x0C))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x04))                              \
        movrs2ld(W(XD), Mebp, inf_SCR02(0x00))                              \
        maxrs2ld(W(XD), Mebp, inf_SCR02(0x04))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x08))                              \
        movrs2ld(W(XD), Mebp, inf_SCR02(0x08))                              \
        maxrs2ld(W(XD), Mebp, inf_SCR02(0x0C))                              \
        movrs2st(W(XD), Mebp, inf_SCR01(0x0C))

#define movrs2st(XS, MD, DD) /* not portable, do not use outside */         \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    REG(XS)))                  \
        EMITW(0x7C00018E | MXM(TmmM,    TEax & M(MOD(MD) == TPxx), TPxx))   \

#define maxrs2ld(XG, MS, DS) /* not portable, do not use outside */         \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DS)), TmmM))                     \
        EMITW(0x1000040A | MXM(REG(XG), REG(XG), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqis_rr(XG, XS)                                                    \
        ceqis3rr(W(XG), W(XG), W(XS))

#define ceqis_ld(XG, MS, DS)                                                \
        ceqis3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqis3rr(XD, XS, XT)                                                \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneis_rr(XG, XS)                                                    \
        cneis3rr(W(XG), W(XG), W(XS))

#define cneis_ld(XG, MS, DS)                                                \
        cneis3ld(W(XG), W(XG), W(MS), W(DS))

#define cneis3rr(XD, XS, XT)                                                \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cneis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltis_rr(XG, XS)                                                    \
        cltis3rr(W(XG), W(XG), W(XS))

#define cltis_ld(XG, MS, DS)                                                \
        cltis3ld(W(XG), W(XG), W(MS), W(DS))

#define cltis3rr(XD, XS, XT)                                                \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100002C6 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define cleis_rr(XG, XS)                                                    \
        cleis3rr(W(XG), W(XG), W(XS))

#define cleis_ld(XG, MS, DS)                                                \
        cleis3ld(W(XG), W(XG), W(MS), W(DS))

#define cleis3rr(XD, XS, XT)                                                \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XT), REG(XS)))

#define cleis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100001C6 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtis_rr(XG, XS)                                                    \
        cgtis3rr(W(XG), W(XG), W(XS))

#define cgtis_ld(XG, MS, DS)                                                \
        cgtis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtis3rr(XD, XS, XT)                                                \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgeis_rr(XG, XS)                                                    \
        cgeis3rr(W(XG), W(XG), W(XS))

#define cgeis_ld(XG, MS, DS)                                                \
        cgeis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeis3rr(XD, XS, XT)                                                \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XS), TmmM))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_128  MN32_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128  MF32_128   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(beq, cr6, lb) ASM_END

#define SMF32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(blt, cr6, lb) ASM_END

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x1000038C | MXM(TmmQ,    0x1F,    0x00))                     \
        EMITW(0x10000486 | MXM(REG(XS), REG(XS), TmmQ))                     \
        AUW(EMPTY, EMPTY, EMPTY, EMPTY, lb,                                 \
        S0(RT_SIMD_MASK_##mask##32_128), EMPTY2)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    TmmM))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x100003CA | MXM(REG(XD), 0x00,    REG(XS)))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x100003CA | MXM(REG(XD), 0x00,    TmmM))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    TmmM))

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
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    REG(XS)))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    TmmM))

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
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    TmmM))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        rnnis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        rnnis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cvz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    REG(XS)))                  \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define rndis_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    TmmM))                     \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define cvtis_rr(XD, XS)                                                    \
        rndis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvtis_ld(XD, MS, DS)                                                \
        rndis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define cvris_rr(XD, XS, mode)                                              \
        rnris_rr(W(XD), W(XS), mode)                                        \
        cvzis_rr(W(XD), W(XD))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x1000034A | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000034A | MXM(REG(XD), 0x00,    TmmM))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtin_rr(XD, XS)                                                    \
        cvnin_rr(W(XD), W(XS))                                            /*!*/

#define cvtin_ld(XD, MS, DS)                                                \
        cvnin_ld(W(XD), W(MS), W(DS))                                     /*!*/

/* cvn (D = unsigned-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnix_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x1000030A | MXM(REG(XD), 0x00,    REG(XS)))

#define cvnix_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000030A | MXM(REG(XD), 0x00,    TmmM))

/* cvt (D = unsigned-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtix_rr(XD, XS)                                                    \
        cvnix_rr(W(XD), W(XS))                                            /*!*/

#define cvtix_ld(XD, MS, DS)                                                \
        cvnix_ld(W(XD), W(MS), W(DS))                                     /*!*/

/* cuz (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    REG(XS)))

#define ruzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000024A | MXM(REG(XD), 0x00,    TmmM))

#define cuzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x1000038A | MXM(REG(XD), 0x00,    REG(XS)))

#define cuzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000038A | MXM(REG(XD), 0x00,    TmmM))

/* cup (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rupis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    REG(XS)))

#define rupis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000028A | MXM(REG(XD), 0x00,    TmmM))

#define cupis_rr(XD, XS)     /* round towards +inf */                       \
        rupis_rr(W(XD), W(XS))                                              \
        cuzis_rr(W(XD), W(XD))

#define cupis_ld(XD, MS, DS) /* round towards +inf */                       \
        rupis_ld(W(XD), W(MS), W(DS))                                       \
        cuzis_rr(W(XD), W(XD))

/* cum (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rumis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    REG(XS)))

#define rumis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x100002CA | MXM(REG(XD), 0x00,    TmmM))

#define cumis_rr(XD, XS)     /* round towards -inf */                       \
        rumis_rr(W(XD), W(XS))                                              \
        cuzis_rr(W(XD), W(XD))

#define cumis_ld(XD, MS, DS) /* round towards -inf */                       \
        rumis_ld(W(XD), W(MS), W(DS))                                       \
        cuzis_rr(W(XD), W(XD))

/* cun (D = fp-to-unsigned-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define runis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)))

#define runis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    TmmM))

#define cunis_rr(XD, XS)     /* round towards near */                       \
        runis_rr(W(XD), W(XS))                                              \
        cuzis_rr(W(XD), W(XD))

#define cunis_ld(XD, MS, DS) /* round towards near */                       \
        runis_ld(W(XD), W(MS), W(DS))                                       \
        cuzis_rr(W(XD), W(XD))

/* cut (D = fp-to-unsigned-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX POWER systems, use cuz
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define rudis_rr(XD, XS)                                                    \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    REG(XS)))                  \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define rudis_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000000A | MXM(REG(XD), TmmR,    TmmM))                     \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XD)))

#define cutis_rr(XD, XS)                                                    \
        rudis_rr(W(XD), W(XS))                                              \
        cuzis_rr(W(XD), W(XD))

#define cutis_ld(XD, MS, DS)                                                \
        rudis_ld(W(XD), W(MS), W(DS))                                       \
        cuzis_rr(W(XD), W(XD))

/* cur (D = fp-to-unsigned-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit unsigned int range */

#define ruris_rr(XD, XS, mode)                                              \
        EMITW(0x1000020A | MXM(REG(XD), 0x00,    REG(XS)) |                 \
        (RT_SIMD_MODE_##mode&3) << 6)

#define curis_rr(XD, XS, mode)                                              \
        ruris_rr(W(XD), W(XS), mode)                                        \
        cuzis_rr(W(XD), W(XD))

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addix_rr(XG, XS)                                                    \
        addix3rr(W(XG), W(XG), W(XS))

#define addix_ld(XG, MS, DS)                                                \
        addix3ld(W(XG), W(XG), W(MS), W(DS))

#define addix3rr(XD, XS, XT)                                                \
        EMITW(0x10000080 | MXM(REG(XD), REG(XS), REG(XT)))

#define addix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000080 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subix_rr(XG, XS)                                                    \
        subix3rr(W(XG), W(XG), W(XS))

#define subix_ld(XG, MS, DS)                                                \
        subix3ld(W(XG), W(XG), W(MS), W(DS))

#define subix3rr(XD, XS, XT)                                                \
        EMITW(0x10000480 | MXM(REG(XD), REG(XS), REG(XT)))

#define subix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000480 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulix_rr(XG, XS)                                                    \
        mulix3rr(W(XG), W(XG), W(XS))

#define mulix_ld(XG, MS, DS)                                                \
        mulix3ld(W(XG), W(XG), W(MS), W(DS))

#define mulix3rr(XD, XS, XT)                                                \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_st(W(XT), Mebp, inf_SCR02(0))                                 \
        mulix_rx(W(XD))

#define mulix3ld(XD, XS, MT, DT)                                            \
        movix_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movix_ld(W(XD), W(MT), W(DT))                                       \
        movix_st(W(XD), Mebp, inf_SCR02(0))                                 \
        mulix_rx(W(XD))

#define mulix_rx(XD) /* not portable, do not use outside */                 \
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
        stack_ld(Recx)                                                      \
        movix_ld(W(XD), Mebp, inf_SCR01(0))

/* shl (G = G << S), (D = S << T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlix_ri(XG, IS)                                                    \
        shlix3ri(W(XG), W(XG), W(IS))

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix3ld(W(XG), W(XG), W(MS), W(DS))

#define shlix3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))

#define shlix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrix_ri(XG, IS)                                                    \
        shrix3ri(W(XG), W(XG), W(IS))

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix3ld(W(XG), W(XG), W(MS), W(DS))

#define shrix3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))

#define shrix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #T) - plain, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrin_ri(XG, IS)                                                    \
        shrin3ri(W(XG), W(XG), W(IS))

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin3ld(W(XG), W(XG), W(MS), W(DS))

#define shrin3ri(XD, XS, IT)                                                \
        EMITW(0x1000038C | MXM(TmmM,    (0x1F & VAL(IT)), 0x00))            \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))

#define shrin3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    TmmM))                     \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix3rr(W(XG), W(XG), W(XS))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix3ld(W(XG), W(XG), W(MS), W(DS))

#define svlix3rr(XD, XS, XT)                                                \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), REG(XT)))

#define svlix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000184 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, unsigned
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix3rr(W(XG), W(XG), W(XS))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix3ld(W(XG), W(XG), W(MS), W(DS))

#define svrix3rr(XD, XS, XT)                                                \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), REG(XT)))

#define svrix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000284 | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #T) - variable, signed
 * for maximum compatibility: shift count must be modulo elem-size */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin3rr(W(XG), W(XG), W(XS))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin3ld(W(XG), W(XG), W(MS), W(DS))

#define svrin3rr(XD, XS, XT)                                                \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), REG(XT)))

#define svrin3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000384 | MXM(REG(XD), REG(XS), TmmM))

/****************   packed single-precision integer compare   *****************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), unsigned */

#define minix_rr(XG, XS)                                                    \
        minix3rr(W(XG), W(XG), W(XS))

#define minix_ld(XG, MS, DS)                                                \
        minix3ld(W(XG), W(XG), W(MS), W(DS))

#define minix3rr(XD, XS, XT)                                                \
        EMITW(0x10000282 | MXM(REG(XD), REG(XS), REG(XT)))

#define minix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000282 | MXM(REG(XD), REG(XS), TmmM))

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T), signed */

#define minin_rr(XG, XS)                                                    \
        minin3rr(W(XG), W(XG), W(XS))

#define minin_ld(XG, MS, DS)                                                \
        minin3ld(W(XG), W(XG), W(MS), W(DS))

#define minin3rr(XD, XS, XT)                                                \
        EMITW(0x10000382 | MXM(REG(XD), REG(XS), REG(XT)))

#define minin3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000382 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), unsigned */

#define maxix_rr(XG, XS)                                                    \
        maxix3rr(W(XG), W(XG), W(XS))

#define maxix_ld(XG, MS, DS)                                                \
        maxix3ld(W(XG), W(XG), W(MS), W(DS))

#define maxix3rr(XD, XS, XT)                                                \
        EMITW(0x10000082 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000082 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T), signed */

#define maxin_rr(XG, XS)                                                    \
        maxin3rr(W(XG), W(XG), W(XS))

#define maxin_ld(XG, MS, DS)                                                \
        maxin3ld(W(XG), W(XG), W(MS), W(DS))

#define maxin3rr(XD, XS, XT)                                                \
        EMITW(0x10000182 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxin3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000182 | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqix_rr(XG, XS)                                                    \
        ceqix3rr(W(XG), W(XG), W(XS))

#define ceqix_ld(XG, MS, DS)                                                \
        ceqix3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqix3rr(XD, XS, XT)                                                \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cneix_rr(XG, XS)                                                    \
        cneix3rr(W(XG), W(XG), W(XS))

#define cneix_ld(XG, MS, DS)                                                \
        cneix3ld(W(XG), W(XG), W(MS), W(DS))

#define cneix3rr(XD, XS, XT)                                                \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cneix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000086 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), unsigned */

#define cltix_rr(XG, XS)                                                    \
        cltix3rr(W(XG), W(XG), W(XS))

#define cltix_ld(XG, MS, DS)                                                \
        cltix3ld(W(XG), W(XG), W(MS), W(DS))

#define cltix3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000286 | MXM(REG(XD), TmmM,    REG(XS)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T), signed */

#define cltin_rr(XG, XS)                                                    \
        cltin3rr(W(XG), W(XG), W(XS))

#define cltin_ld(XG, MS, DS)                                                \
        cltin3ld(W(XG), W(XG), W(MS), W(DS))

#define cltin3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltin3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000386 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), unsigned */

#define cleix_rr(XG, XS)                                                    \
        cleix3rr(W(XG), W(XG), W(XS))

#define cleix_ld(XG, MS, DS)                                                \
        cleix3ld(W(XG), W(XG), W(MS), W(DS))

#define cleix3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cleix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T), signed */

#define clein_rr(XG, XS)                                                    \
        clein3rr(W(XG), W(XG), W(XS))

#define clein_ld(XG, MS, DS)                                                \
        clein3ld(W(XG), W(XG), W(MS), W(DS))

#define clein3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define clein3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), unsigned */

#define cgtix_rr(XG, XS)                                                    \
        cgtix3rr(W(XG), W(XG), W(XS))

#define cgtix_ld(XG, MS, DS)                                                \
        cgtix3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtix3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000286 | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T), signed */

#define cgtin_rr(XG, XS)                                                    \
        cgtin3rr(W(XG), W(XG), W(XS))

#define cgtin_ld(XG, MS, DS)                                                \
        cgtin3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtin3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtin3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000386 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), unsigned */

#define cgeix_rr(XG, XS)                                                    \
        cgeix3rr(W(XG), W(XG), W(XS))

#define cgeix_ld(XG, MS, DS)                                                \
        cgeix3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeix3rr(XD, XS, XT)                                                \
        EMITW(0x10000286 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cgeix3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000286 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T), signed */

#define cgein_rr(XG, XS)                                                    \
        cgein3rr(W(XG), W(XG), W(XS))

#define cgein_ld(XG, MS, DS)                                                \
        cgein3ld(W(XG), W(XG), W(MS), W(DS))

#define cgein3rr(XD, XS, XT)                                                \
        EMITW(0x10000386 | MXM(REG(XD), REG(XT), REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cgein3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C0000CE | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x10000386 | MXM(REG(XD), TmmM,    REG(XS)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*********   scalar single-precision floating-point move/arithmetic   *********/

#if RT_ELEM_COMPAT_VMX == 0

/* mov (D = S) */

#define movrs_rr(XD, XS)                                                    \
        EMITW(0xFC000090 | MXM(REG(XD), 0x00,    REG(XS)))

#define movrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(XD), MOD(MS), VAL(DS), B1(DS), L1(DS)))

#define movrs_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(XS), MOD(MD), VAL(DD), B1(DD), U1(DD)))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addrs_rr(XG, XS)                                                    \
        addrs3rr(W(XG), W(XG), W(XS))

#define addrs_ld(XG, MS, DS)                                                \
        addrs3ld(W(XG), W(XG), W(MS), W(DS))

#define addrs3rr(XD, XS, XT)                                                \
        EMITW(0xEC00002A | MXM(REG(XD), REG(XS), REG(XT)))

#define addrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), L1(DT)))  \
        EMITW(0xEC00002A | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subrs_rr(XG, XS)                                                    \
        subrs3rr(W(XG), W(XG), W(XS))

#define subrs_ld(XG, MS, DS)                                                \
        subrs3ld(W(XG), W(XG), W(MS), W(DS))

#define subrs3rr(XD, XS, XT)                                                \
        EMITW(0xEC000028 | MXM(REG(XD), REG(XS), REG(XT)))

#define subrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), L1(DT)))  \
        EMITW(0xEC000028 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulrs_rr(XG, XS)                                                    \
        mulrs3rr(W(XG), W(XG), W(XS))

#define mulrs_ld(XG, MS, DS)                                                \
        mulrs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulrs3rr(XD, XS, XT)                                                \
        EMITW(0xEC000032 | MXM(REG(XD), REG(XS), 0x00) | REG(XT) << 6)

#define mulrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), L1(DT)))  \
        EMITW(0xEC000032 | MXM(REG(XD), REG(XS), 0x00) | TmmM << 6)

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divrs_rr(XG, XS)                                                    \
        divrs3rr(W(XG), W(XG), W(XS))

#define divrs_ld(XG, MS, DS)                                                \
        divrs3ld(W(XG), W(XG), W(MS), W(DS))

#define divrs3rr(XD, XS, XT)                                                \
        EMITW(0xEC000024 | MXM(REG(XD), REG(XS), REG(XT)))

#define divrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), L1(DT)))  \
        EMITW(0xEC000024 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
        EMITW(0xEC00002C | MXM(REG(XD), 0x00,    REG(XS)))

#define sqrrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TmmM,    MOD(MS), VAL(DS), B1(DS), L1(DS)))  \
        EMITW(0xEC00002C | MXM(REG(XD), 0x00,    TmmM))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
        movrs_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_ld(W(XD), Mebp, inf_SCR02(0))

#define rcsrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
        sqrrs_rr(W(XD), W(XS))                                              \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movrs_ld(W(XD), Mebp, inf_GPC01_32)                                 \
        divrs_ld(W(XD), Mebp, inf_SCR02(0))

#define rssrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMITW(0xEC00003A | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), L1(DT)))  \
        EMITW(0xEC00003A | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMITW(0xEC00003C | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TmmM,    MOD(MT), VAL(DT), B1(DT), L1(DT)))  \
        EMITW(0xEC00003C | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMS */

#else  /* RT_ELEM_COMPAT_VMX == 1, -- only if BASE regs are 128bit-aligned -- */

/* mov (D = S) */

#define movrs_rr(XD, XS)                                                    \
        EMITW(0x10000484 | MXM(REG(XD), REG(XS), REG(XS)))

#define movrs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(REG(XD), TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(REG(XD), SPL(W(DS)), REG(XD)))

#define movrs_st(XS, MD, DD)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MD), VAL(DD), B2(DD), P2(DD)))  \
        EMITW(0x1000028C | MXM(TmmM,    SPLT,    REG(XS)))                  \
        EMITW(0x7C00018E | MXM(TmmM,    TEax & M(MOD(MD) == TPxx), TPxx))

/* add (G = G + S), (D = S + T) if (#D != #T) */

#define addrs_rr(XG, XS)                                                    \
        addrs3rr(W(XG), W(XG), W(XS))

#define addrs_ld(XG, MS, DS)                                                \
        addrs3ld(W(XG), W(XG), W(MS), W(DS))

#define addrs3rr(XD, XS, XT)                                                \
        EMITW(0x1000000A | MXM(REG(XD), REG(XS), REG(XT)))

#define addrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000000A | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #T) */

#define subrs_rr(XG, XS)                                                    \
        subrs3rr(W(XG), W(XG), W(XS))

#define subrs_ld(XG, MS, DS)                                                \
        subrs3ld(W(XG), W(XG), W(MS), W(DS))

#define subrs3rr(XD, XS, XT)                                                \
        EMITW(0x1000004A | MXM(REG(XD), REG(XS), REG(XT)))

#define subrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000004A | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #T) */

#define mulrs_rr(XG, XS)                                                    \
        mulrs3rr(W(XG), W(XG), W(XS))

#define mulrs_ld(XG, MS, DS)                                                \
        mulrs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulrs3rr(XD, XS, XT)                                                \
        EMITW(0x1000002E | MXM(REG(XD), REG(XS), TmmS) | REG(XT) << 6)

#define mulrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000002E | MXM(REG(XD), REG(XS), TmmS) | TmmM << 6)

/* div (G = G / S), (D = S / T) if (#D != #T) and on ARMv7 if (#D != #S) */

#define divrs_rr(XG, XS)                                                    \
        divrs3rr(W(XG), W(XG), W(XS))

#define divrs_ld(XG, MS, DS)                                                \
        divrs3ld(W(XG), W(XG), W(MS), W(DS))

#if RT_SIMD_COMPAT_DIV == 1

#define divrs3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define divrs3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movws_ld(Tff2,  Mebp, inf_SCR02(0x00))                              \
        divws_rr(Tff1, Tff2)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_DIV */

#define divrs3rr(XD, XS, XT)                                                \
        EMITW(0x1000010A | MXM(TmmW,    0x00,    REG(XT)))                  \
        EMITW(0x1000002F | MXM(TmmZ,    TmmW,    TmmU) | REG(XT) << 6)      \
        EMITW(0x1000002E | MXM(TmmW,    TmmW,    TmmW) | TmmZ << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XS), TmmS) | TmmW << 6)         \
        EMITW(0x1000002F | MXM(REG(XD), TmmZ, REG(XS)) | REG(XT) << 6)      \
        EMITW(0x1000002E | MXM(REG(XD), REG(XD), TmmZ) | TmmW << 6)

#define divrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000010A | MXM(TmmW,    0x00,    TmmM))                     \
        EMITW(0x1000002F | MXM(TmmZ,    TmmW,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002E | MXM(TmmW,    TmmW,    TmmW) | TmmZ << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XS), TmmS) | TmmW << 6)         \
        EMITW(0x1000002F | MXM(REG(XD), TmmZ, REG(XS)) | TmmM << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), REG(XD), TmmZ) | TmmW << 6)

#endif /* RT_SIMD_COMPAT_DIV */

/* sqr (D = sqrt S) */

#if RT_SIMD_COMPAT_SQR == 1

#define sqrrs_rr(XD, XS)                                                    \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define sqrrs_ld(XD, MS, DS)                                                \
        movrs_ld(W(XD), W(MS), W(DS))                                       \
        movrs_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movws_ld(Tff1,  Mebp, inf_SCR01(0x00))                              \
        sqrws_rr(Tff1, Tff1)                                                \
        movws_st(Tff1,  Mebp, inf_SCR01(0x00))                              \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#else /* RT_SIMD_COMPAT_SQR */

#define sqrrs_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(TmmW,    0x00,    REG(XS)))                  \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmW,    TmmS) | REG(XS) << 6)

#define sqrrs_ld(XD, MS, DS)                                                \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MS) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000014A | MXM(TmmW,    0x00,    TmmM))                     \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(TmmZ,    TmmW,    TmmS) | TmmW << 6)         \
        EMITW(0x1000002E | MXM(TmmT,    TmmW,    TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | TmmM << 6)         \
        EMITW(0x1000002F | MXM(TmmW,    TmmZ,    TmmW) | TmmT << 6)         \
        EMITW(0x1000002E | MXM(REG(XD), TmmW,    TmmS) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_SQR */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
        EMITW(0x1000010A | MXM(REG(XD), 0x00,    REG(XS)))

#define rcsrs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1000002F | MXM(REG(XS), REG(XG), TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002E | MXM(REG(XG), REG(XG), REG(XG)) | REG(XS) << 6)

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
        EMITW(0x1000014A | MXM(REG(XD), 0x00,    REG(XS)))

#define rssrs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1000002E | MXM(TmmZ,    REG(XG), TmmS) | REG(XG) << 6)      \
        EMITW(0x1000002E | MXM(TmmW,    REG(XG), TmmS) | TmmV << 6)         \
        EMITW(0x1000002F | MXM(TmmZ,    TmmZ,    TmmU) | REG(XS) << 6)      \
        EMITW(0x1000002F | MXM(REG(XG), TmmZ,    REG(XG)) | TmmW << 6)

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000002E | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | REG(XT) << 6)

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000002F | MXM(REG(XG), REG(XS), REG(XG)) | TmmM << 6)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_ELEM_COMPAT_VMX */

/*************   scalar single-precision floating-point compare   *************/

#if RT_ELEM_COMPAT_VMX == 0

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minrs_rr(XG, XS)                                                    \
        minrs3rr(W(XG), W(XG), W(XS))

#define minrs_ld(XG, MS, DS)                                                \
        minrs3ld(W(XG), W(XG), W(MS), W(DS))

#define minrs3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        minis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define minrs3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        minis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxrs_rr(XG, XS)                                                    \
        maxrs3rr(W(XG), W(XG), W(XS))

#define maxrs_ld(XG, MS, DS)                                                \
        maxrs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxrs3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        maxis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        maxis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqrs_rr(XG, XS)                                                    \
        ceqrs3rr(W(XG), W(XG), W(XS))

#define ceqrs_ld(XG, MS, DS)                                                \
        ceqrs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqrs3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        ceqis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define ceqrs3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        ceqis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cners_rr(XG, XS)                                                    \
        cners3rr(W(XG), W(XG), W(XS))

#define cners_ld(XG, MS, DS)                                                \
        cners3ld(W(XG), W(XG), W(MS), W(DS))

#define cners3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cneis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define cners3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cneis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltrs_rr(XG, XS)                                                    \
        cltrs3rr(W(XG), W(XG), W(XS))

#define cltrs_ld(XG, MS, DS)                                                \
        cltrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltrs3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cltis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define cltrs3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cltis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clers_rr(XG, XS)                                                    \
        clers3rr(W(XG), W(XG), W(XS))

#define clers_ld(XG, MS, DS)                                                \
        clers3ld(W(XG), W(XG), W(MS), W(DS))

#define clers3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cleis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define clers3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cleis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtrs_rr(XG, XS)                                                    \
        cgtrs3rr(W(XG), W(XG), W(XS))

#define cgtrs_ld(XG, MS, DS)                                                \
        cgtrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtrs3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cgtis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define cgtrs3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cgtis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgers_rr(XG, XS)                                                    \
        cgers3rr(W(XG), W(XG), W(XS))

#define cgers_ld(XG, MS, DS)                                                \
        cgers3ld(W(XG), W(XG), W(MS), W(DS))

#define cgers3rr(XD, XS, XT)                                                \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_st(W(XT), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cgeis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#define cgers3ld(XD, XS, MT, DT)                                            \
        movrs_st(W(XS), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), W(MT), W(DT))                                       \
        movrs_st(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XD), Mebp, inf_SCR01(0))                                 \
        cgeis_ld(W(XD), Mebp, inf_SCR02(0))                                 \
        movix_st(W(XD), Mebp, inf_SCR01(0))                                 \
        movrs_ld(W(XD), Mebp, inf_SCR01(0))

#else  /* RT_ELEM_COMPAT_VMX == 1, -- only if BASE regs are 128bit-aligned -- */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #T) */

#define minrs_rr(XG, XS)                                                    \
        minrs3rr(W(XG), W(XG), W(XS))

#define minrs_ld(XG, MS, DS)                                                \
        minrs3ld(W(XG), W(XG), W(MS), W(DS))

#define minrs3rr(XD, XS, XT)                                                \
        EMITW(0x1000044A | MXM(REG(XD), REG(XS), REG(XT)))

#define minrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000044A | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #T) */

#define maxrs_rr(XG, XS)                                                    \
        maxrs3rr(W(XG), W(XG), W(XS))

#define maxrs_ld(XG, MS, DS)                                                \
        maxrs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxrs3rr(XD, XS, XT)                                                \
        EMITW(0x1000040A | MXM(REG(XD), REG(XS), REG(XT)))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x1000040A | MXM(REG(XD), REG(XS), TmmM))

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #T) */

#define ceqrs_rr(XG, XS)                                                    \
        ceqrs3rr(W(XG), W(XG), W(XS))

#define ceqrs_ld(XG, MS, DS)                                                \
        ceqrs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqrs3rr(XD, XS, XT)                                                \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #T) */

#define cners_rr(XG, XS)                                                    \
        cners3rr(W(XG), W(XG), W(XS))

#define cners_ld(XG, MS, DS)                                                \
        cners3ld(W(XG), W(XG), W(MS), W(DS))

#define cners3rr(XD, XS, XT)                                                \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

#define cners3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x100000C6 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x10000504 | MXM(REG(XD), REG(XD), REG(XD)))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #T) */

#define cltrs_rr(XG, XS)                                                    \
        cltrs3rr(W(XG), W(XG), W(XS))

#define cltrs_ld(XG, MS, DS)                                                \
        cltrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltrs3rr(XD, XS, XT)                                                \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x100002C6 | MXM(REG(XD), TmmM,    REG(XS)))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #T) */

#define clers_rr(XG, XS)                                                    \
        clers3rr(W(XG), W(XG), W(XS))

#define clers_ld(XG, MS, DS)                                                \
        clers3ld(W(XG), W(XG), W(MS), W(DS))

#define clers3rr(XD, XS, XT)                                                \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XT), REG(XS)))

#define clers3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x100001C6 | MXM(REG(XD), TmmM,    REG(XS)))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #T) */

#define cgtrs_rr(XG, XS)                                                    \
        cgtrs3rr(W(XG), W(XG), W(XS))

#define cgtrs_ld(XG, MS, DS)                                                \
        cgtrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtrs3rr(XD, XS, XT)                                                \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtrs3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x100002C6 | MXM(REG(XD), REG(XS), TmmM))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #T) */

#define cgers_rr(XG, XS)                                                    \
        cgers3rr(W(XG), W(XG), W(XS))

#define cgers_ld(XG, MS, DS)                                                \
        cgers3ld(W(XG), W(XG), W(MS), W(DS))

#define cgers3rr(XD, XS, XT)                                                \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgers3ld(XD, XS, MT, DT)                                            \
        AUW(EMPTY,    EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x38000000 | MPM(TPxx,    REG(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7C00008E | MXM(TmmM,    TEax & M(MOD(MT) == TPxx), TPxx))   \
        EMITW(0x1000028C | MXM(TmmM, SPL(W(DT)), TmmM))                     \
        EMITW(0x100001C6 | MXM(REG(XD), REG(XS), TmmM))

#endif /* RT_ELEM_COMPAT_VMX */

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
#define RT_SIMD_MODE_ROUNDM     0x03    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x02    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x01    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x05    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x07    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x06    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x05    /* round towards zero */

#define fpscr_ld(RS) /* not portable, do not use outside */                 \
        EMITW(0xFE00058E | MRM(0x00,    REG(RS), 0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xFC00048E | MRM(REG(RD), 0x00,    0x00))

#define F0(mode)    FT(mode)
#define FT(mode)    F##mode
#define F0x00                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmS))
#define F0x03                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmV))
#define F0x02                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmV))
#define F0x01                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmS)) /*!*/
#define F0x04                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmS))
#define F0x07                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmV))
#define F0x06                   EMITW(0x1000004A | MXM(TmmR, TmmS, TmmV))
#define F0x05                   EMITW(0x1000000A | MXM(TmmR, TmmS, TmmS)) /*!*/

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        F0(RT_SIMD_MODE_##mode)

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        F0(RT_SIMD_MODE_ROUNDN)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#ifndef RT_RTARCH_P32_128X2VG_H
#undef  RT_128X2
#define RT_128X2  16
#include "rtarch_p32_128x2vG.h"
#endif /* RT_RTARCH_P32_128X2VG_H */

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_P32_128X1V4_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

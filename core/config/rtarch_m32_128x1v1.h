/******************************************************************************/
/* Copyright (c) 2013-2018 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_128X1V1_H
#define RT_RTARCH_M32_128X1V1_H

#include "rtarch_m64.h"

#define RT_SIMD_REGS_128        32

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32_128x1v1.h: Implementation of MIPS fp32 MSA instructions.
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
 * should propagate QNaNs unchanged, however this behavior hasn't been verified.
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

#if (RT_128X1 != 0)

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 11 | (reg) << 6)

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 11 | (reg) << 6)

#define MFM(reg, brm, vdp, bxx, fxx)                                        \
        (fxx(vdp) | bxx(brm) << 11 | (reg) << 6 | RT_SIMD_COMPAT_D12)

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  B4(val, tp1, tp2)  B4##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  F2(val, tp1, tp2)  F2##tp2
#define  L2(val, tp1, tp2)  L2##tp2
#define  K2(val, tp1, tp2)  K2##tp2
#define  C2(val, tp1, tp2)  C2##tp2
#define  A2(val, tp1, tp2)  A2##tp2

/* displacement encoding SIMD(TP2), ELEM(TP1) */

#define B20(br) (br)
#define B40(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0xFF8) << 13)
#define F20(dp) (0x01FF0000 & ((dp) & 0xFFC) << (14 - RT_SIMD_COMPAT_D12))
#define L20(dp) (0x00000000 | ((dp) & 0xFF8) << 13)
#define K20(dp) (0x01FF0000 & ((dp) & 0xFFC) << (14 - RT_SIMD_COMPAT_D12))
#define C20(br, dp) EMPTY
#define A20(br, dp) EMPTY

#define B21(br) (br)
#define B41(br) TPxx
#define P21(dp) (0x00000000)
#define F21(dp) (0x00000000)
#define L21(dp) (0x00000000 | ((dp) & 0x010) << 13)
#define K21(dp) (0x01FF0000 & ((dp) & 0x010) << (14 - RT_SIMD_COMPAT_D12))
#define C21(br, dp) EMITW(0x34000000 | TDxx << 16 | (0xFFFC & (dp)))
#define A21(br, dp) C21(br, dp)                                             \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx) | ADR)

#define B22(br) (br)
#define B42(br) TPxx
#define P22(dp) (0x00000000)
#define F22(dp) (0x00000000)
#define L22(dp) (0x00000000 | ((dp) & 0x010) << 13)
#define K22(dp) (0x01FF0000 & ((dp) & 0x010) << (14 - RT_SIMD_COMPAT_D12))
#define C22(br, dp) EMITW(0x3C000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x34000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFFC & (dp)))
#define A22(br, dp) C22(br, dp)                                             \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx) | ADR)

/* configuration for vector/scalar compatibility mode */

#if RT_ENDIAN == 1 && RT_SIMD_COMPAT_D12 != 0 && RT_ELEM_COMPAT_MSA == 0
#define SBF(x)  x
#define SBX(x)
#else /* RT_ENDIAN, RT_SIMD_COMPAT_D12, RT_ELEM_COMPAT_MSA */
#define SBF(x)
#define SBX(x)  x
#endif /* RT_ENDIAN, RT_SIMD_COMPAT_D12, RT_ELEM_COMPAT_MSA */

#if RT_ENDIAN == 1 && RT_SIMD_COMPAT_D12 != 0 && RT_ELEM_COMPAT_MSA != 0
#define SHF(x)  x
#define SHX(x)
#else /* RT_ENDIAN, RT_SIMD_COMPAT_D12, RT_ELEM_COMPAT_MSA */
#define SHF(x)
#define SHX(x)  x
#endif /* RT_ENDIAN, RT_SIMD_COMPAT_D12, RT_ELEM_COMPAT_MSA */

#if RT_ENDIAN == 1 && RT_SIMD_COMPAT_D12 != 0 && RT_ELEM_COMPAT_MSA != 0 && 0
#define SJF(x)  x
#define SJX(x)
#else /* RT_ENDIAN, RT_SIMD_COMPAT_D12, RT_ELEM_COMPAT_MSA */
#define SJF(x)
#define SJX(x)  x
#endif /* RT_ENDIAN, RT_SIMD_COMPAT_D12, RT_ELEM_COMPAT_MSA */

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tmm0    0x00  /* w0,  internal name for Xmm0 (in mmv) */
#define TmmZ    0x0F  /* w15, zero-mask all 0s, TmmZ (in sregs) */
#define TmmM    0x1F  /* w31, temp-reg name for mem-args */

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/* registers    REG,  MOD,  SIB */

#define Xmm0    0x00, $w0,  EMPTY       /* w0 */
#define Xmm1    0x01, $w1,  EMPTY       /* w1 */
#define Xmm2    0x02, $w2,  EMPTY       /* w2 */
#define Xmm3    0x03, $w3,  EMPTY       /* w3 */
#define Xmm4    0x04, $w4,  EMPTY       /* w4 */
#define Xmm5    0x05, $w5,  EMPTY       /* w5 */
#define Xmm6    0x06, $w6,  EMPTY       /* w6 */
#define Xmm7    0x07, $w7,  EMPTY       /* w7 */
#define Xmm8    0x08, $w8,  EMPTY       /* w8 */
#define Xmm9    0x09, $w9,  EMPTY       /* w9 */
#define XmmA    0x0A, $w10, EMPTY       /* w10 */
#define XmmB    0x0B, $w11, EMPTY       /* w11 */
#define XmmC    0x0C, $w12, EMPTY       /* w12 */
#define XmmD    0x0D, $w13, EMPTY       /* w13 */
#define XmmE    0x0E, $w14, EMPTY       /* w14 */

/* only for 128-bit instructions (save/restore in 256-bit header)
 * provided as an extension to common baseline of 15 registers */

#define XmmF    0x1E, $w30, EMPTY       /* w30 */
#define XmmG    0x10, $w16, EMPTY       /* w16 */
#define XmmH    0x11, $w17, EMPTY       /* w17 */
#define XmmI    0x12, $w18, EMPTY       /* w18 */
#define XmmJ    0x13, $w19, EMPTY       /* w19 */
#define XmmK    0x14, $w20, EMPTY       /* w20 */
#define XmmL    0x15, $w21, EMPTY       /* w21 */
#define XmmM    0x16, $w22, EMPTY       /* w22 */
#define XmmN    0x17, $w23, EMPTY       /* w23 */
#define XmmO    0x18, $w24, EMPTY       /* w24 */
#define XmmP    0x19, $w25, EMPTY       /* w25 */
#define XmmQ    0x1A, $w26, EMPTY       /* w26 */
#define XmmR    0x1B, $w27, EMPTY       /* w27 */
#define XmmS    0x1C, $w28, EMPTY       /* w28 */
#define XmmT    0x1D, $w29, EMPTY       /* w29 */

/******************************************************************************/
/**********************************   SIMD   **********************************/
/******************************************************************************/

/* elm (D = S), store first SIMD element with natural alignment
 * allows to decouple scalar subset from SIMD where appropriate */

#define elmix_st(XS, MD, DD) /* 1st elem as in mem with SIMD load/store */  \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
    SBF(EMITW(0x7AB10002 | MXM(TmmM,    REG(XS), 0x00)))                    \
    SBF(EMITW(0xE4000000 | MDM(TmmM,    MOD(MD), VAL(DD), B3(DD), P1(DD)))) \
    SBX(EMITW(0xE4000000 | MDM(REG(XS), MOD(MD), VAL(DD), B3(DD), P1(DD))))

/***************   packed single-precision generic move/logic   ***************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMITW(0x78BE0019 | MXM(REG(XD), REG(XS), 0x00))

#define movix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(REG(XD), MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(REG(XD), REG(XD), 0x00)))

#define movix_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A2(DD), EMPTY2)   \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    REG(XS), 0x00)))                    \
    SHF(EMITW(0x78000026 | MFM(TmmM,    MOD(MD), VAL(DD), B4(DD), F2(DD)))) \
    SHX(EMITW(0x78000026 | MFM(REG(XS), MOD(MD), VAL(DD), B4(DD), F2(DD))))

/* mmv (G = G mask-merge S) where (mask-elem: 0 keeps G, -1 picks S)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked elems */

#define mmvix_rr(XG, XS)                                                    \
        EMITW(0x7880001E | MXM(REG(XG), REG(XS), Tmm0))

#define mmvix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001E | MXM(REG(XG), TmmM,    Tmm0))

#define mmvix_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), A2(DG), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MG), VAL(DG), B4(DG), F2(DG)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001E | MXM(TmmM,    REG(XS), Tmm0))                     \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78000026 | MFM(TmmM,    MOD(MG), VAL(DG), B4(DG), F2(DG)))

/* and (G = G & S), (D = S & T) if (#D != #S) */

#define andix_rr(XG, XS)                                                    \
        andix3rr(W(XG), W(XG), W(XS))

#define andix_ld(XG, MS, DS)                                                \
        andix3ld(W(XG), W(XG), W(MS), W(DS))

#define andix3rr(XD, XS, XT)                                                \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), REG(XT)))

#define andix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7800001E | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (#D != #S) */

#define annix_rr(XG, XS)                                                    \
        EMITW(0x78C0001E | MXM(REG(XG), REG(XS), TmmZ))

#define annix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001E | MXM(REG(XG), TmmM,    TmmZ))

#define annix3rr(XD, XS, XT)                                                \
        movix_rr(W(XD), W(XS))                                              \
        annix_rr(W(XD), W(XT))

#define annix3ld(XD, XS, MT, DT)                                            \
        movix_rr(W(XD), W(XS))                                              \
        annix_ld(W(XD), W(MT), W(DT))

/* orr (G = G | S), (D = S | T) if (#D != #S) */

#define orrix_rr(XG, XS)                                                    \
        orrix3rr(W(XG), W(XG), W(XS))

#define orrix_ld(XG, MS, DS)                                                \
        orrix3ld(W(XG), W(XG), W(MS), W(DS))

#define orrix3rr(XD, XS, XT)                                                \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), REG(XT)))

#define orrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7820001E | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (#D != #S) */

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

/* xor (G = G ^ S), (D = S ^ T) if (#D != #S) */

#define xorix_rr(XG, XS)                                                    \
        xorix3rr(W(XG), W(XG), W(XS))

#define xorix_ld(XG, MS, DS)                                                \
        xorix3ld(W(XG), W(XG), W(MS), W(DS))

#define xorix3rr(XD, XS, XT)                                                \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), REG(XT)))

#define xorix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G), (D = ~S) */

#define notix_rx(XG)                                                        \
        notix_rr(W(XG), W(XG))

#define notix_rr(XD, XS)                                                    \
        EMITW(0x7840001E | MXM(REG(XD), TmmZ,    REG(XS)))

/************   packed single-precision floating-point arithmetic   ***********/

/* neg (G = -G), (D = -S) */

#define negis_rx(XG)                                                        \
        negis_rr(W(XG), W(XG))

#define negis_rr(XD, XS)                                                    \
        movix_xm(Mebp, inf_GPC06_32)                                        \
        EMITW(0x7860001E | MXM(REG(XD), REG(XS), TmmM))

#define movix_xm(MS, DS) /* not portable, do not use outside */             \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addis_rr(XG, XS)                                                    \
        addis3rr(W(XG), W(XG), W(XS))

#define addis_ld(XG, MS, DS)                                                \
        addis3ld(W(XG), W(XG), W(MS), W(DS))

#define addis3rr(XD, XS, XT)                                                \
        EMITW(0x7800001B | MXM(REG(XD), REG(XS), REG(XT)))

#define addis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7800001B | MXM(REG(XD), REG(XS), TmmM))

        /* adp, adh are defined in rtbase.h (first 15-regs only)
         * under "COMMON SIMD INSTRUCTIONS" section */

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subis_rr(XG, XS)                                                    \
        subis3rr(W(XG), W(XG), W(XS))

#define subis_ld(XG, MS, DS)                                                \
        subis3ld(W(XG), W(XG), W(MS), W(DS))

#define subis3rr(XD, XS, XT)                                                \
        EMITW(0x7840001B | MXM(REG(XD), REG(XS), REG(XT)))

#define subis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840001B | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulis_rr(XG, XS)                                                    \
        mulis3rr(W(XG), W(XG), W(XS))

#define mulis_ld(XG, MS, DS)                                                \
        mulis3ld(W(XG), W(XG), W(MS), W(DS))

#define mulis3rr(XD, XS, XT)                                                \
        EMITW(0x7880001B | MXM(REG(XD), REG(XS), REG(XT)))

#define mulis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001B | MXM(REG(XD), REG(XS), TmmM))

        /* mlp, mlh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divis_rr(XG, XS)                                                    \
        divis3rr(W(XG), W(XG), W(XS))

#define divis_ld(XG, MS, DS)                                                \
        divis3ld(W(XG), W(XG), W(MS), W(DS))

#define divis3rr(XD, XS, XT)                                                \
        EMITW(0x78C0001B | MXM(REG(XD), REG(XS), REG(XT)))

#define divis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001B | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqris_rr(XD, XS)                                                    \
        EMITW(0x7B26001E | MXM(REG(XD), REG(XS), 0x00))

#define sqris_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B26001E | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITW(0x7B2A001E | MXM(REG(XD), REG(XS), 0x00))

#define rcsis_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITW(0x7B28001E | MXM(REG(XD), REG(XS), 0x00))

#define rssis_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/*************   packed single-precision floating-point compare   *************/

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minis_rr(XG, XS)                                                    \
        minis3rr(W(XG), W(XG), W(XS))

#define minis_ld(XG, MS, DS)                                                \
        minis3ld(W(XG), W(XG), W(MS), W(DS))

#define minis3rr(XD, XS, XT)                                                \
        EMITW(0x7B00001B | MXM(REG(XD), REG(XS), REG(XT)))

#define minis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B00001B | MXM(REG(XD), REG(XS), TmmM))

        /* mnp, mnh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxis_rr(XG, XS)                                                    \
        maxis3rr(W(XG), W(XG), W(XS))

#define maxis_ld(XG, MS, DS)                                                \
        maxis3ld(W(XG), W(XG), W(MS), W(DS))

#define maxis3rr(XD, XS, XT)                                                \
        EMITW(0x7B80001B | MXM(REG(XD), REG(XS), REG(XT)))

#define maxis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B80001B | MXM(REG(XD), REG(XS), TmmM))

        /* mxp, mxh are defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqis_rr(XG, XS)                                                    \
        ceqis3rr(W(XG), W(XG), W(XS))

#define ceqis_ld(XG, MS, DS)                                                \
        ceqis3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqis3rr(XD, XS, XT)                                                \
        EMITW(0x7880001A | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7880001A | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cneis_rr(XG, XS)                                                    \
        cneis3rr(W(XG), W(XG), W(XS))

#define cneis_ld(XG, MS, DS)                                                \
        cneis3ld(W(XG), W(XG), W(MS), W(DS))

#define cneis3rr(XD, XS, XT)                                                \
        EMITW(0x78C0001C | MXM(REG(XD), REG(XS), REG(XT)))

#define cneis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0001C | MXM(REG(XD), REG(XS), TmmM))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltis_rr(XG, XS)                                                    \
        cltis3rr(W(XG), W(XG), W(XS))

#define cltis_ld(XG, MS, DS)                                                \
        cltis3ld(W(XG), W(XG), W(MS), W(DS))

#define cltis3rr(XD, XS, XT)                                                \
        EMITW(0x7900001A | MXM(REG(XD), REG(XS), REG(XT)))

#define cltis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001A | MXM(REG(XD), REG(XS), TmmM))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define cleis_rr(XG, XS)                                                    \
        cleis3rr(W(XG), W(XG), W(XS))

#define cleis_ld(XG, MS, DS)                                                \
        cleis3ld(W(XG), W(XG), W(MS), W(DS))

#define cleis3rr(XD, XS, XT)                                                \
        EMITW(0x7980001A | MXM(REG(XD), REG(XS), REG(XT)))

#define cleis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7980001A | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtis_rr(XG, XS)                                                    \
        cgtis3rr(W(XG), W(XG), W(XS))

#define cgtis_ld(XG, MS, DS)                                                \
        cgtis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtis3rr(XD, XS, XT)                                                \
        EMITW(0x7900001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7900001A | MXM(REG(XD), TmmM,    REG(XS)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgeis_rr(XG, XS)                                                    \
        cgeis3rr(W(XG), W(XG), W(XS))

#define cgeis_ld(XG, MS, DS)                                                \
        cgeis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeis3rr(XD, XS, XT)                                                \
        EMITW(0x7980001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7980001A | MXM(REG(XD), TmmM,    REG(XS)))

/* mkj (jump to lb) if (S satisfies mask condition) */

#define RT_SIMD_MASK_NONE32_128  MN32_128   /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128  MF32_128   /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask

#define SMN32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2( bz.v, xs, lb) ASM_END

#define SMF32_128(xs, lb) /* not portable, do not use outside */            \
        ASM_BEG ASM_OP2(bnz.w, xs, lb) ASM_END

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        AUW(EMPTY, EMPTY, EMPTY, MOD(XS), lb,                               \
        S0(RT_SIMD_MASK_##mask##32_128), EMPTY2)

/*************   packed single-precision floating-point convert   *************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        cvzis_rr(W(XD), W(XS))                                              \
        cvnin_rr(W(XD), W(XD))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        cvzis_ld(W(XD), W(MS), W(DS))                                       \
        cvnin_rr(W(XD), W(XD))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x7B22001E | MXM(REG(XD), REG(XS), 0x00))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B22001E | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtis_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        rndis_rr(W(XD), W(XS))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        rndis_ld(W(XD), W(MS), W(DS))

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
        EMITW(0x7B2C001E | MXM(REG(XD), REG(XS), 0x00))

#define rndis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B2C001E | MXM(REG(XD), TmmM,    0x00))

#define cvtis_rr(XD, XS)                                                    \
        EMITW(0x7B38001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B38001E | MXM(REG(XD), TmmM,    0x00))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX POWER systems */

#define cvtin_rr(XD, XS)                                                    \
        EMITW(0x7B3C001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtin_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A2(DS), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MS), VAL(DS), B4(DS), F2(DS)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7B3C001E | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, fp32 SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvris_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

/************   packed single-precision integer arithmetic/shifts   ***********/

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addix_rr(XG, XS)                                                    \
        addix3rr(W(XG), W(XG), W(XS))

#define addix_ld(XG, MS, DS)                                                \
        addix3ld(W(XG), W(XG), W(MS), W(DS))

#define addix3rr(XD, XS, XT)                                                \
        EMITW(0x7840000E | MXM(REG(XD), REG(XS), REG(XT)))

#define addix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000E | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subix_rr(XG, XS)                                                    \
        subix3rr(W(XG), W(XG), W(XS))

#define subix_ld(XG, MS, DS)                                                \
        subix3ld(W(XG), W(XG), W(MS), W(DS))

#define subix3rr(XD, XS, XT)                                                \
        EMITW(0x78C0000E | MXM(REG(XD), REG(XS), REG(XT)))

#define subix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0000E | MXM(REG(XD), REG(XS), TmmM))

/* shl (G = G << S), (D = S << T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlix_ri(XG, IS)                                                    \
        shlix3ri(W(XG), W(XG), W(IS))

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shlix3ld(W(XG), W(XG), W(MS), W(DS))

#define shlix3ri(XD, XS, IT)                                                \
        EMITW(0x78400009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shlix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7840000D | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrix_ri(XG, IS)                                                    \
        shrix3ri(W(XG), W(XG), W(IS))

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrix3ld(W(XG), W(XG), W(MS), W(DS))

#define shrix3ri(XD, XS, IT)                                                \
        EMITW(0x79400009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7940000D | MXM(REG(XD), REG(XS), TmmM))

/* shr (G = G >> S), (D = S >> T) if (#D != #S) - plain, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrin_ri(XG, IS)                                                    \
        shrin3ri(W(XG), W(XG), W(IS))

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses first elem, rest zeroed */ \
        shrin3ld(W(XG), W(XG), W(MS), W(DS))

#define shrin3ri(XD, XS, IT)                                                \
        EMITW(0x78C00009 | MXM(REG(XD), REG(XS), 0x00) |                    \
                                                 (0x1F & VAL(IT)) << 16)

#define shrin3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x78C0000D | MXM(REG(XD), REG(XS), TmmM))

/* svl (G = G << S), (D = S << T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svlix3rr(W(XG), W(XG), W(XS))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svlix3ld(W(XG), W(XG), W(MS), W(DS))

#define svlix3rr(XD, XS, XT)                                                \
        EMITW(0x7840000D | MXM(REG(XD), REG(XS), REG(XT)))

#define svlix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7840000D | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, unsigned
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrix3rr(W(XG), W(XG), W(XS))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrix3ld(W(XG), W(XG), W(MS), W(DS))

#define svrix3rr(XD, XS, XT)                                                \
        EMITW(0x7940000D | MXM(REG(XD), REG(XS), REG(XT)))

#define svrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x7940000D | MXM(REG(XD), REG(XS), TmmM))

/* svr (G = G >> S), (D = S >> T) if (#D != #S) - variable, signed
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        svrin3rr(W(XG), W(XG), W(XS))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        svrin3ld(W(XG), W(XG), W(MS), W(DS))

#define svrin3rr(XD, XS, XT)                                                \
        EMITW(0x78C0000D | MXM(REG(XD), REG(XS), REG(XT)))

#define svrin3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A2(DT), EMPTY2)   \
        EMITW(0x78000022 | MFM(TmmM,    MOD(MT), VAL(DT), B4(DT), F2(DT)))  \
    SHF(EMITW(0x7AB10002 | MXM(TmmM,    TmmM,    0x00)))                    \
        EMITW(0x78C0000D | MXM(REG(XD), REG(XS), TmmM))

/******************************************************************************/
/**********************************   ELEM   **********************************/
/******************************************************************************/

/*********   scalar single-precision floating-point move/arithmetic   *********/

/* mov (D = S) */

#define movrs_rr(XD, XS)                                                    \
        EMITW(0x46000006 | MXM(REG(XD), REG(XS), 0x00))

#define movrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xC4000000 | MDM(REG(XD), MOD(MS), VAL(DS), B3(DS), P1(DS)))

#define movrs_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMITW(0xE4000000 | MDM(REG(XS), MOD(MD), VAL(DD), B3(DD), P1(DD)))

/* add (G = G + S), (D = S + T) if (#D != #S) */

#define addrs_rr(XG, XS)                                                    \
        addrs3rr(W(XG), W(XG), W(XS))

#define addrs_ld(XG, MS, DS)                                                \
        addrs3ld(W(XG), W(XG), W(MS), W(DS))

#define addrs3rr(XD, XS, XT)                                                \
        EMITW(0x46000000 | MXM(REG(XD), REG(XS), REG(XT)))

#define addrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x46000000 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (#D != #S) */

#define subrs_rr(XG, XS)                                                    \
        subrs3rr(W(XG), W(XG), W(XS))

#define subrs_ld(XG, MS, DS)                                                \
        subrs3ld(W(XG), W(XG), W(MS), W(DS))

#define subrs3rr(XD, XS, XT)                                                \
        EMITW(0x46000001 | MXM(REG(XD), REG(XS), REG(XT)))

#define subrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x46000001 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (#D != #S) */

#define mulrs_rr(XG, XS)                                                    \
        mulrs3rr(W(XG), W(XG), W(XS))

#define mulrs_ld(XG, MS, DS)                                                \
        mulrs3ld(W(XG), W(XG), W(MS), W(DS))

#define mulrs3rr(XD, XS, XT)                                                \
        EMITW(0x46000002 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x46000002 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (#D != #S) */

#define divrs_rr(XG, XS)                                                    \
        divrs3rr(W(XG), W(XG), W(XS))

#define divrs_ld(XG, MS, DS)                                                \
        divrs3ld(W(XG), W(XG), W(MS), W(DS))

#define divrs3rr(XD, XS, XT)                                                \
        EMITW(0x46000003 | MXM(REG(XD), REG(XS), REG(XT)))

#define divrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x46000003 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
        EMITW(0x46000004 | MXM(REG(XD), REG(XS), 0x00))

#define sqrrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MS), VAL(DS), B3(DS), P1(DS)))  \
        EMITW(0x46000004 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
        EMITW(0x46000015 | MXM(REG(XD), REG(XS), 0x00))

#define rcsrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rce, rcs, rcp are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
        EMITW(0x46000016 | MXM(REG(XD), REG(XS), 0x00))

#define rssrs_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rse, rss, rsq are defined in rtconf.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_BASE_COMPAT_REV < 6) /* pre-r6 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_BASE_COMPAT_REV >= 6 : r6 */

/* fma (G = G + S * T) if (#G != #S && #G != #T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMITW(0x46000018 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x46000018 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T) if (#G != #S && #G != #T)
 * NOTE: due to final negation being outside of rounding on all POWER systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMITW(0x46000019 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x46000019 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_BASE_COMPAT_REV >= 6 : r6 */

/*************   scalar single-precision floating-point compare   *************/

#if (RT_BASE_COMPAT_REV < 6) /* pre-r6 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minrs_rr(XG, XS)                                                    \
        minrs3rr(W(XG), W(XG), W(XS))

#define minrs_ld(XG, MS, DS)                                                \
        minrs3ld(W(XG), W(XG), W(MS), W(DS))

#define minrs3rr(XD, XS, XT)                                                \
        EMITW(0x7B00001B | MXM(REG(XD), REG(XS), REG(XT)))

#define minrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B00001B | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxrs_rr(XG, XS)                                                    \
        maxrs3rr(W(XG), W(XG), W(XS))

#define maxrs_ld(XG, MS, DS)                                                \
        maxrs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxrs3rr(XD, XS, XT)                                                \
        EMITW(0x7B80001B | MXM(REG(XD), REG(XS), REG(XT)))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7B80001B | MXM(REG(XD), REG(XS), TmmM))

#else /* RT_BASE_COMPAT_REV >= 6 : r6 */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (#D != #S) */

#define minrs_rr(XG, XS)                                                    \
        minrs3rr(W(XG), W(XG), W(XS))

#define minrs_ld(XG, MS, DS)                                                \
        minrs3ld(W(XG), W(XG), W(MS), W(DS))

#define minrs3rr(XD, XS, XT)                                                \
        EMITW(0x4600001C | MXM(REG(XD), REG(XS), REG(XT)))

#define minrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x4600001C | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (#D != #S) */

#define maxrs_rr(XG, XS)                                                    \
        maxrs3rr(W(XG), W(XG), W(XS))

#define maxrs_ld(XG, MS, DS)                                                \
        maxrs3ld(W(XG), W(XG), W(MS), W(DS))

#define maxrs3rr(XD, XS, XT)                                                \
        EMITW(0x4600001E | MXM(REG(XD), REG(XS), REG(XT)))

#define maxrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x4600001E | MXM(REG(XD), REG(XS), TmmM))

#endif /* RT_BASE_COMPAT_REV >= 6 : r6 */

/* ceq (G = G == S ? -1 : 0), (D = S == T ? -1 : 0) if (#D != #S) */

#define ceqrs_rr(XG, XS)                                                    \
        ceqrs3rr(W(XG), W(XG), W(XS))

#define ceqrs_ld(XG, MS, DS)                                                \
        ceqrs3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqrs3rr(XD, XS, XT)                                                \
        EMITW(0x7880001A | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7880001A | MXM(REG(XD), REG(XS), TmmM))

/* cne (G = G != S ? -1 : 0), (D = S != T ? -1 : 0) if (#D != #S) */

#define cners_rr(XG, XS)                                                    \
        cners3rr(W(XG), W(XG), W(XS))

#define cners_ld(XG, MS, DS)                                                \
        cners3ld(W(XG), W(XG), W(MS), W(DS))

#define cners3rr(XD, XS, XT)                                                \
        EMITW(0x78C0001C | MXM(REG(XD), REG(XS), REG(XT)))

#define cners3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x78C0001C | MXM(REG(XD), REG(XS), TmmM))

/* clt (G = G < S ? -1 : 0), (D = S < T ? -1 : 0) if (#D != #S) */

#define cltrs_rr(XG, XS)                                                    \
        cltrs3rr(W(XG), W(XG), W(XS))

#define cltrs_ld(XG, MS, DS)                                                \
        cltrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cltrs3rr(XD, XS, XT)                                                \
        EMITW(0x7900001A | MXM(REG(XD), REG(XS), REG(XT)))

#define cltrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7900001A | MXM(REG(XD), REG(XS), TmmM))

/* cle (G = G <= S ? -1 : 0), (D = S <= T ? -1 : 0) if (#D != #S) */

#define clers_rr(XG, XS)                                                    \
        clers3rr(W(XG), W(XG), W(XS))

#define clers_ld(XG, MS, DS)                                                \
        clers3ld(W(XG), W(XG), W(MS), W(DS))

#define clers3rr(XD, XS, XT)                                                \
        EMITW(0x7980001A | MXM(REG(XD), REG(XS), REG(XT)))

#define clers3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7980001A | MXM(REG(XD), REG(XS), TmmM))

/* cgt (G = G > S ? -1 : 0), (D = S > T ? -1 : 0) if (#D != #S) */

#define cgtrs_rr(XG, XS)                                                    \
        cgtrs3rr(W(XG), W(XG), W(XS))

#define cgtrs_ld(XG, MS, DS)                                                \
        cgtrs3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtrs3rr(XD, XS, XT)                                                \
        EMITW(0x7900001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgtrs3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7900001A | MXM(REG(XD), TmmM,    REG(XS)))

/* cge (G = G >= S ? -1 : 0), (D = S >= T ? -1 : 0) if (#D != #S) */

#define cgers_rr(XG, XS)                                                    \
        cgers3rr(W(XG), W(XG), W(XS))

#define cgers_ld(XG, MS, DS)                                                \
        cgers3ld(W(XG), W(XG), W(MS), W(DS))

#define cgers3rr(XD, XS, XT)                                                \
        EMITW(0x7980001A | MXM(REG(XD), REG(XT), REG(XS)))

#define cgers3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMITW(0xC4000000 | MDM(TmmM,    MOD(MT), VAL(DT), B3(DT), P1(DT)))  \
        EMITW(0x7980001A | MXM(REG(XD), TmmM,    REG(XS)))

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
        EMITW(0x783E0019 | MXM(0x01,    REG(RS), 0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0x787E0019 | MXM(REG(RD), 0x01,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0x34000000 | TNxx << 21 | TIxx << 16 |                        \
                           (RT_SIMD_MODE_##mode&3))                         \
        EMITW(0x783E0019 | MXM(0x01,    TIxx,    0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0x783E0019 | MXM(0x01,    TNxx,    0x00))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0x783E0019 | MXM(0x01, TNxx+(RT_SIMD_MODE_##mode&3), 0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0x783E0019 | MXM(0x01,    TNxx,    0x00))

#endif /* RT_SIMD_FAST_FCTRL */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#ifndef RT_RTARCH_M64_128X2V1_H
#undef  RT_128X2
#define RT_128X2  RT_128X1
#include "rtarch_m64_128x2v1.h"
#endif /* RT_RTARCH_M64_128X2V1_H */

#endif /* RT_128X1 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M32_128X1V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

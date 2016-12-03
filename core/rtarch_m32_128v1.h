/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_128V1_H
#define RT_RTARCH_M32_128V1_H

#include "rtarch_m64.h"

#define RT_SIMD_REGS_128        16
#define RT_SIMD_ALIGN_128       16
#define RT_SIMD_WIDTH64_128     2
#define RT_SIMD_SET64_128(s, v) s[0]=s[1]=v
#define RT_SIMD_WIDTH32_128     4
#define RT_SIMD_SET32_128(s, v) s[0]=s[1]=s[2]=s[3]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32_128v1.h: Implementation of MIPS fp32 MSA instructions.
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

#undef  sregs_sa
#undef  sregs_la

/* structural */

#define MXM(reg, ren, rem)                                                  \
        ((rem) << 16 | (ren) << 11 | (reg) << 6)

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 11 | (reg) << 6)

/* selectors  */

#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0xFF0) << 13)
#define C20(br, dp) EMPTY

#define B21(br) TPxx
#define P21(dp) (0x00000000)
#define C21(br, dp) EMITW(0x34000000 | TDxx << 16 | (0xFFF0 & (dp)))        \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx) | ADR)

#define B22(br) TPxx
#define P22(dp) (0x00000000)
#define C22(br, dp) EMITW(0x3C000000 | TDxx << 16 | (0x7FFF & (dp) >> 16))  \
                    EMITW(0x34000000 | TDxx << 16 | TDxx << 21 |            \
                                                    (0xFFF0 & (dp)))        \
                    EMITW(0x00000021 | MRM(TPxx,    (br),    TDxx) | ADR)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tmm0    0x00  /* w0,  internal name for Xmm0 (in mmv) */
#define TmmE    0x0E  /* w14, internal name for XmmE (in sregs) */
#define TmmF    0x10  /* w16, internal name for XmmF (in sregs) */
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
#if     RT_SIMD_COMPAT_XMM < 2
#define XmmE    TmmE, $w14, EMPTY       /* w14, may be reserved in some cases */
#if     RT_SIMD_COMPAT_XMM < 1
#define XmmF    TmmF, $w16, EMPTY       /* w16, may be reserved in some cases */
#endif/*RT_SIMD_COMPAT_XMM < 1*/
#endif/*RT_SIMD_COMPAT_XMM < 2*/

/* The last two SIMD registers can be reserved by the assembler when building
 * RISC targets with SIMD wider than natively supported 128-bit, in which case
 * they will be occupied by temporary data. Two hidden registers may also come
 * in handy when implementing elaborate register-spill techniques in the future
 * for current targets with less native registers than architecturally exposed.
 *
 * It should be possible to reserve only 1 SIMD register (XmmF) to achieve the
 * goals above (totalling 15 regs) at the cost of extra loads in certain ops. */

/******************************************************************************/
/**********************************   MSA   ***********************************/
/******************************************************************************/

/* adr (D = adr S) */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, MS/DS is SIMD-aligned */  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x00000021 | MRM(REG(RD), MOD(MS), TDxx) | ADR)

/**************************   packed generic (SIMD)   *************************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMITW(0x78BE0019 | MXM(REG(XD), REG(XS), 0x00))

#define movix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movix_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x78000027 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvix_rr(XG, XS)                                                    \
        EMITW(0x7880001E | MXM(REG(XG), REG(XS), Tmm0))

#define mmvix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7880001E | MXM(REG(XG), TmmM,    Tmm0))

#define mmvix_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x7880001E | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x78000027 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* and (G = G & S) */

#define andix_rr(XG, XS)                                                    \
        EMITW(0x7800001E | MXM(REG(XG), REG(XG), REG(XS)))

#define andix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7800001E | MXM(REG(XG), REG(XG), TmmM))

/* ann (G = ~G & S) */

#define annix_rr(XG, XS)                                                    \
        EMITW(0x78C0001E | MXM(REG(XG), REG(XS), TmmZ))

#define annix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001E | MXM(REG(XG), TmmM,    TmmZ))

/* orr (G = G | S) */

#define orrix_rr(XG, XS)                                                    \
        EMITW(0x7820001E | MXM(REG(XG), REG(XG), REG(XS)))

#define orrix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7820001E | MXM(REG(XG), REG(XG), TmmM))

/* orn (G = ~G | S) */

#define ornix_rr(XG, XS)                                                    \
        notix_rx(W(XG))                                                     \
        orrix_rr(W(XG), W(XS))

#define ornix_ld(XG, MS, DS)                                                \
        notix_rx(W(XG))                                                     \
        orrix_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorix_rr(XG, XS)                                                    \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), REG(XS)))

#define xorix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), TmmM))

/* not (G = ~G) */

#define notix_rx(XG)                                                        \
        EMITW(0x7840001E | MXM(REG(XG), TmmZ,    REG(XG)))

/**************   packed single precision floating point (SIMD)   *************/

/* neg (G = -G) */

#define negis_rx(XG)                                                        \
        movix_xm(Mebp, inf_GPC06_32)                                        \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), TmmM))

#define movix_xm(MS, DS) /* not portable, do not use outside */             \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))

/* add (G = G + S) */

#define addis_rr(XG, XS)                                                    \
        EMITW(0x7800001B | MXM(REG(XG), REG(XG), REG(XS)))

#define addis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7800001B | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subis_rr(XG, XS)                                                    \
        EMITW(0x7840001B | MXM(REG(XG), REG(XG), REG(XS)))

#define subis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7840001B | MXM(REG(XG), REG(XG), TmmM))

/* mul (G = G * S) */

#define mulis_rr(XG, XS)                                                    \
        EMITW(0x7880001B | MXM(REG(XG), REG(XG), REG(XS)))

#define mulis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7880001B | MXM(REG(XG), REG(XG), TmmM))

/* div (G = G / S) */

#define divis_rr(XG, XS)                                                    \
        EMITW(0x78C0001B | MXM(REG(XG), REG(XG), REG(XS)))

#define divis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001B | MXM(REG(XG), REG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqris_rr(XD, XS)                                                    \
        EMITW(0x7B26001E | MXM(REG(XD), REG(XS), 0x00))

#define sqris_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B26001E | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITW(0x7B2A001E | MXM(REG(XD), REG(XS), 0x00))

#define rcsis_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITW(0x7B28001E | MXM(REG(XD), REG(XS), 0x00))

#define rssis_rr(XG, XS) /* destroys XS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minis_rr(XG, XS)                                                    \
        EMITW(0x7B00001B | MXM(REG(XG), REG(XG), REG(XS)))

#define minis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B00001B | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxis_rr(XG, XS)                                                    \
        EMITW(0x7B80001B | MXM(REG(XG), REG(XG), REG(XS)))

#define maxis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B80001B | MXM(REG(XG), REG(XG), TmmM))

/* cmp (G = G ? S) */

#define ceqis_rr(XG, XS)                                                    \
        EMITW(0x7880001A | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7880001A | MXM(REG(XG), REG(XG), TmmM))

#define cneis_rr(XG, XS)                                                    \
        EMITW(0x78C0001C | MXM(REG(XG), REG(XG), REG(XS)))

#define cneis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001C | MXM(REG(XG), REG(XG), TmmM))

#define cltis_rr(XG, XS)                                                    \
        EMITW(0x7900001A | MXM(REG(XG), REG(XG), REG(XS)))

#define cltis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7900001A | MXM(REG(XG), REG(XG), TmmM))

#define cleis_rr(XG, XS)                                                    \
        EMITW(0x7980001A | MXM(REG(XG), REG(XG), REG(XS)))

#define cleis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7980001A | MXM(REG(XG), REG(XG), TmmM))

#define cgtis_rr(XG, XS)                                                    \
        EMITW(0x7900001A | MXM(REG(XG), REG(XS), REG(XG)))

#define cgtis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7900001A | MXM(REG(XG), TmmM,    REG(XG)))

#define cgeis_rr(XG, XS)                                                    \
        EMITW(0x7980001A | MXM(REG(XG), REG(XS), REG(XG)))

#define cgeis_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7980001A | MXM(REG(XG), TmmM,    REG(XG)))

/**************************   packed integer (SIMD)   *************************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B22001E | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
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

/* add (G = G + S) */

#define addix_rr(XG, XS)                                                    \
        EMITW(0x7840000E | MXM(REG(XG), REG(XG), REG(XS)))

#define addix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7840000E | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subix_rr(XG, XS)                                                    \
        EMITW(0x78C0000E | MXM(REG(XG), REG(XG), REG(XS)))

#define subix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0000E | MXM(REG(XG), REG(XG), TmmM))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlix_ri(XG, IS)                                                    \
        EMITW(0x78400009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7840000D | MXM(REG(XG), REG(XG), TmmM))

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x7840000D | MXM(REG(XG), REG(XG), REG(XS)))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7840000D | MXM(REG(XG), REG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrix_ri(XG, IS)                                                    \
        EMITW(0x79400009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x7940000D | MXM(REG(XG), REG(XG), TmmM))

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x7940000D | MXM(REG(XG), REG(XG), REG(XS)))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7940000D | MXM(REG(XG), REG(XG), TmmM))


#define shrin_ri(XG, IS)                                                    \
        EMITW(0x78C00009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B02001E | MXM(TmmM,    TMxx,    0x00))                     \
        EMITW(0x78C0000D | MXM(REG(XG), REG(XG), TmmM))

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x78C0000D | MXM(REG(XG), REG(XG), REG(XS)))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0000D | MXM(REG(XG), REG(XG), TmmM))

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

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

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
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

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        EMITW(0x7B2C001E | MXM(REG(XD), REG(XS), 0x00))

#define rndis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B2C001E | MXM(REG(XD), TmmM,    0x00))

#define cvtis_rr(XD, XS)                                                    \
        EMITW(0x7B38001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B38001E | MXM(REG(XD), TmmM,    0x00))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtin_rr(XD, XS)                                                    \
        EMITW(0x7B3C001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtin_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B3C001E | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvris_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtis_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_st(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000027 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000027 | MXM(TmmZ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000027 | MXM(TmmF,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000027 | MXM(TmmM,    Teax,    0x00))

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movix_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm7, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm8, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(Xmm9, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmA, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmB, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmC, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        movix_ld(XmmD, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000023 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000023 | MXM(TmmZ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000023 | MXM(TmmF,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x78000023 | MXM(TmmM,    Teax,    0x00))

#ifndef RT_RTARCH_M64_256V1_H
#undef  RT_256
#define RT_256  (RT_128)
#include "rtarch_m64_256v1.h"
#endif /* RT_RTARCH_M64_256V1_H */

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M32_128V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

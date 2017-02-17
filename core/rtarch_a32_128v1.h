/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_128V1_H
#define RT_RTARCH_A32_128V1_H

#include "rtarch_a64.h"

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
 * rtarch_a32_128v1.h: Implementation of AArch64 fp32 NEON instructions.
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
        ((rem) << 16 | (ren) << 5 | (reg))

#define MPM(reg, brm, vdp, bxx, pxx)                                        \
        (pxx(vdp) | bxx(brm) << 5 | (reg))

/* selectors  */

#define VSL(val, tp1, tp2)  ((val) << 2)
#define  B2(val, tp1, tp2)  B2##tp2
#define  P2(val, tp1, tp2)  P2##tp2
#define  C2(val, tp1, tp2)  C2##tp2

/* displacement encoding SIMD(TP2) */

#define B20(br) (br)
#define P20(dp) (0x00000000 | ((dp) & 0xFFF0) << 6)
#define C20(br, dp) EMPTY

#define B22(br) TPxx
#define P22(dp) (0x00000000)
#define C22(br, dp) EMITW(0x52800000 | MXM(TDxx,    0x00,    0x00) |        \
                             (0xFFF0 & (dp)) << 5)                          \
                    EMITW(0x72A00000 | MXM(TDxx,    0x00,    0x00) |        \
                             (0x7FFF & (dp) >> 16) << 5)                    \
                    EMITW(0x0B000000 | MXM(TPxx,    (br),    TDxx) | ADR)

/* registers    REG   (check mapping with ASM_ENTER/ASM_LEAVE in rtarch.h) */

#define Tmm0    0x00  /* v0,  internal name for Xmm0 (in mmv) */
#define TmmE    0x0E  /* v14, internal name for XmmE (in sregs) */
#define TmmF    0x0F  /* v15, internal name for XmmF (in sregs) */
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
#define Xmm8    0x08, 0x00, EMPTY       /* v8 */
#define Xmm9    0x09, 0x00, EMPTY       /* v9 */
#define XmmA    0x0A, 0x00, EMPTY       /* v10 */
#define XmmB    0x0B, 0x00, EMPTY       /* v11 */
#define XmmC    0x0C, 0x00, EMPTY       /* v12 */
#define XmmD    0x0D, 0x00, EMPTY       /* v13 */
#if     RT_SIMD_COMPAT_XMM < 2
#define XmmE    0x0E, 0x00, EMPTY       /* v14, may be reserved in some cases */
#if     RT_SIMD_COMPAT_XMM < 1
#define XmmF    0x0F, 0x00, EMPTY       /* v15, may be reserved in some cases */
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
/**********************************   MPE   ***********************************/
/******************************************************************************/

/* adr (D = adr S) */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, MS/DS is SIMD-aligned */  \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x8B000000 | MRM(REG(RD), MOD(MS), TDxx))

/**************************   packed generic (NEON)   *************************/

/* mov (D = S) */

#define movix_rr(XD, XS)                                                    \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XS)))

#define movix_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movix_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x3D800000 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvix_rr(XG, XS)                                                    \
        EMITW(0x6EA01C00 | MXM(REG(XG), REG(XS), Tmm0))

#define mmvix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA01C00 | MXM(REG(XG), TmmM,    Tmm0))

#define mmvix_st(XS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C2(DG), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))  \
        EMITW(0x6EA01C00 | MXM(TmmM,    REG(XS), Tmm0))                     \
        EMITW(0x3D800000 | MPM(TmmM,    MOD(MG), VAL(DG), B2(DG), P2(DG)))

/* and (G = G & S), (D = S & T) if (D != S) */

#define andix_rr(XG, XS)                                                    \
        andix3rr(W(XG), W(XG), W(XS))

#define andix_ld(XG, MS, DS)                                                \
        andix3ld(W(XG), W(XG), W(MS), W(DS))

#define andix3rr(XD, XS, XT)                                                \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define andix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* ann (G = ~G & S), (D = ~S & T) if (D != S) */

#define annix_rr(XG, XS)                                                    \
        annix3rr(W(XG), W(XG), W(XS))

#define annix_ld(XG, MS, DS)                                                \
        annix3ld(W(XG), W(XG), W(MS), W(DS))

#define annix3rr(XD, XS, XT)                                                \
        EMITW(0x4E601C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define annix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E601C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* orr (G = G | S), (D = S | T) if (D != S) */

#define orrix_rr(XG, XS)                                                    \
        orrix3rr(W(XG), W(XG), W(XS))

#define orrix_ld(XG, MS, DS)                                                \
        orrix3ld(W(XG), W(XG), W(MS), W(DS))

#define orrix3rr(XD, XS, XT)                                                \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define orrix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA01C00 | MXM(REG(XD), REG(XS), TmmM))

/* orn (G = ~G | S), (D = ~S | T) if (D != S) */

#define ornix_rr(XG, XS)                                                    \
        ornix3rr(W(XG), W(XG), W(XS))

#define ornix_ld(XG, MS, DS)                                                \
        ornix3ld(W(XG), W(XG), W(MS), W(DS))

#define ornix3rr(XD, XS, XT)                                                \
        EMITW(0x4EE01C00 | MXM(REG(XD), REG(XT), REG(XS)))

#define ornix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EE01C00 | MXM(REG(XD), TmmM,    REG(XS)))

/* xor (G = G ^ S), (D = S ^ T) if (D != S) */

#define xorix_rr(XG, XS)                                                    \
        xorix3rr(W(XG), W(XG), W(XS))

#define xorix_ld(XG, MS, DS)                                                \
        xorix3ld(W(XG), W(XG), W(MS), W(DS))

#define xorix3rr(XD, XS, XT)                                                \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), REG(XT)))

#define xorix3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E201C00 | MXM(REG(XD), REG(XS), TmmM))

/* not (G = ~G) */

#define notix_rx(XG)                                                        \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

/**************   packed single precision floating point (NEON)   *************/

/* neg (G = -G) */

#define negis_rx(XG)                                                        \
        EMITW(0x6EA0F800 | MXM(REG(XG), REG(XG), 0x00))

/* add (G = G + S), (D = S + T) if (D != S) */

#define addis_rr(XG, XS)                                                    \
        addis3rr(W(XG), W(XG), W(XS))

#define addis_ld(XG, MS, DS)                                                \
        addis3ld(W(XG), W(XG), W(MS), W(DS))

#define addis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20D400 | MXM(REG(XD), REG(XS), REG(XT)))

#define addis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20D400 | MXM(REG(XD), REG(XS), TmmM))

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subis_rr(XG, XS)                                                    \
        subis3rr(W(XG), W(XG), W(XS))

#define subis_ld(XG, MS, DS)                                                \
        subis3ld(W(XG), W(XG), W(MS), W(DS))

#define subis3rr(XD, XS, XT)                                                \
        EMITW(0x4EA0D400 | MXM(REG(XD), REG(XS), REG(XT)))

#define subis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0D400 | MXM(REG(XD), REG(XS), TmmM))

/* mul (G = G * S), (D = S * T) if (D != S) */

#define mulis_rr(XG, XS)                                                    \
        mulis3rr(W(XG), W(XG), W(XS))

#define mulis_ld(XG, MS, DS)                                                \
        mulis3ld(W(XG), W(XG), W(MS), W(DS))

#define mulis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20DC00 | MXM(REG(XD), REG(XS), REG(XT)))

#define mulis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20DC00 | MXM(REG(XD), REG(XS), TmmM))

/* div (G = G / S), (D = S / T) if (D != S) */

#define divis_rr(XG, XS)                                                    \
        divis3rr(W(XG), W(XG), W(XS))

#define divis_ld(XG, MS, DS)                                                \
        divis3ld(W(XG), W(XG), W(MS), W(DS))

#define divis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20FC00 | MXM(REG(XD), REG(XS), REG(XT)))

#define divis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20FC00 | MXM(REG(XD), REG(XS), TmmM))

/* sqr (D = sqrt S) */

#define sqris_rr(XD, XS)                                                    \
        EMITW(0x6EA1F800 | MXM(REG(XD), REG(XS), 0x00))

#define sqris_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA1F800 | MXM(REG(XD), TmmM,    0x00))

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceis_rr(XD, XS)                                                    \
        EMITW(0x4EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x4E20FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseis_rr(XD, XS)                                                    \
        EMITW(0x6EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssis_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x6E20DC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x4EA0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x6E20DC00 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmais_rr(XG, XS, XT)                                                \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmais_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20CC00 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsis_rr(XG, XS, XT)                                                \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsis_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0CC00 | MXM(REG(XG), REG(XS), TmmM))

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S), (D = S < T ? S : T) if (D != S) */

#define minis_rr(XG, XS)                                                    \
        minis3rr(W(XG), W(XG), W(XS))

#define minis_ld(XG, MS, DS)                                                \
        minis3ld(W(XG), W(XG), W(MS), W(DS))

#define minis3rr(XD, XS, XT)                                                \
        EMITW(0x4EA0F400 | MXM(REG(XD), REG(XS), REG(XT)))

#define minis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4EA0F400 | MXM(REG(XD), REG(XS), TmmM))

/* max (G = G > S ? G : S), (D = S > T ? S : T) if (D != S) */

#define maxis_rr(XG, XS)                                                    \
        maxis3rr(W(XG), W(XG), W(XS))

#define maxis_ld(XG, MS, DS)                                                \
        maxis3ld(W(XG), W(XG), W(MS), W(DS))

#define maxis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20F400 | MXM(REG(XD), REG(XS), REG(XT)))

#define maxis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20F400 | MXM(REG(XD), REG(XS), TmmM))

/* cmp (G = G ? S), (D = S ? T) if (D != S) */

#define ceqis_rr(XG, XS)                                                    \
        ceqis3rr(W(XG), W(XG), W(XS))

#define ceqis_ld(XG, MS, DS)                                                \
        ceqis3ld(W(XG), W(XG), W(MS), W(DS))

#define ceqis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define ceqis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), TmmM))


#define cneis_rr(XG, XS)                                                    \
        cneis3rr(W(XG), W(XG), W(XS))

#define cneis_ld(XG, MS, DS)                                                \
        cneis3ld(W(XG), W(XG), W(MS), W(DS))

#define cneis3rr(XD, XS, XT)                                                \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), REG(XT)))                  \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))

#define cneis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x4E20E400 | MXM(REG(XD), REG(XS), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XD), REG(XD), 0x00))


#define cltis_rr(XG, XS)                                                    \
        cltis3rr(W(XG), W(XG), W(XS))

#define cltis_ld(XG, MS, DS)                                                \
        cltis3ld(W(XG), W(XG), W(MS), W(DS))

#define cltis3rr(XD, XS, XT)                                                \
        EMITW(0x6EA0E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cltis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA0E400 | MXM(REG(XD), TmmM,    REG(XS)))


#define cleis_rr(XG, XS)                                                    \
        cleis3rr(W(XG), W(XG), W(XS))

#define cleis_ld(XG, MS, DS)                                                \
        cleis3ld(W(XG), W(XG), W(MS), W(DS))

#define cleis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20E400 | MXM(REG(XD), REG(XT), REG(XS)))

#define cleis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20E400 | MXM(REG(XD), TmmM,    REG(XS)))


#define cgtis_rr(XG, XS)                                                    \
        cgtis3rr(W(XG), W(XG), W(XS))

#define cgtis_ld(XG, MS, DS)                                                \
        cgtis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgtis3rr(XD, XS, XT)                                                \
        EMITW(0x6EA0E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgtis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6EA0E400 | MXM(REG(XD), REG(XS), TmmM))


#define cgeis_rr(XG, XS)                                                    \
        cgeis3rr(W(XG), W(XG), W(XS))

#define cgeis_ld(XG, MS, DS)                                                \
        cgeis3ld(W(XG), W(XG), W(MS), W(DS))

#define cgeis3rr(XD, XS, XT)                                                \
        EMITW(0x6E20E400 | MXM(REG(XD), REG(XS), REG(XT)))

#define cgeis3ld(XD, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x6E20E400 | MXM(REG(XD), REG(XS), TmmM))

/**************************   packed integer (NEON)   *************************/

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA19800 | MXM(REG(XD), REG(XS), 0x00))

#define rnzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA19800 | MXM(REG(XD), TmmM,    0x00))

#define cvzis_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x4EA1B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvzis_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1B800 | MXM(REG(XD), TmmM,    0x00))

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA18800 | MXM(REG(XD), REG(XS), 0x00))

#define rnpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA18800 | MXM(REG(XD), TmmM,    0x00))

#define cvpis_rr(XD, XS)     /* round towards +inf */                       \
        EMITW(0x4EA1A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvpis_ld(XD, MS, DS) /* round towards +inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA1A800 | MXM(REG(XD), TmmM,    0x00))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E219800 | MXM(REG(XD), REG(XS), 0x00))

#define rnmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E219800 | MXM(REG(XD), TmmM,    0x00))

#define cvmis_rr(XD, XS)     /* round towards -inf */                       \
        EMITW(0x4E21B800 | MXM(REG(XD), REG(XS), 0x00))

#define cvmis_ld(XD, MS, DS) /* round towards -inf */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21B800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00))

#define rnnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E218800 | MXM(REG(XD), TmmM,    0x00))

#define cvnis_rr(XD, XS)     /* round towards near */                       \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00))

#define cvnis_ld(XD, MS, DS) /* round towards near */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21A800 | MXM(REG(XD), TmmM,    0x00))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnin_rr(XD, XS)     /* round towards near */                       \
        cvtin_rr(W(XD), W(XS))

#define cvnin_ld(XD, MS, DS) /* round towards near */                       \
        cvtin_ld(W(XD), W(MS), W(DS))

/* add (G = G + S) */

#define addix_rr(XG, XS)                                                    \
        EMITW(0x4EA08400 | MXM(REG(XG), REG(XG), REG(XS)))

#define addix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4EA08400 | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subix_rr(XG, XS)                                                    \
        EMITW(0x6EA08400 | MXM(REG(XG), REG(XG), REG(XS)))

#define subix_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA08400 | MXM(REG(XG), REG(XG), TmmM))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlix_ri(XG, IS)                                                    \
        EMITW(0x4F205400 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shlix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), TmmM))

#define svlix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), REG(XS)))

#define svlix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), TmmM))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrix_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F200400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shrix_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), TmmM))

#define svrix_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EA0B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), TmmM))

#define svrix_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA04400 | MXM(REG(XG), REG(XG), TmmM))


#define shrin_ri(XG, IS) /* emits shift-left for zero-immediate args */     \
        EMITW(0x4F200400 | MXM(REG(XG), REG(XG), 0x00) |                    \
        (+(VAL(IS) == 0) & 0x00005000) | (+(VAL(IS) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IS)) << 16)

#define shrin_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E040400 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XG), REG(XG), TmmM))

#define svrin_rr(XG, XS)     /* variable shift with per-elem count */       \
        EMITW(0x6EA0B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XG), REG(XG), TmmM))

#define svrin_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA0B800 | MXM(TmmM,    TmmM,    0x00))                     \
        EMITW(0x4EA04400 | MXM(REG(XG), REG(XG), TmmM))

/**************************   helper macros (NEON)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE32_128     0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_128     0x04    /*  all satisfy the condition */

#define mkjix_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        EMITW(0x4EB1B800 | MXM(TmmM,    REG(XS), 0x00))                     \
        EMITW(0x0E043C00 | MXM(Teax,    TmmM,    0x00))                     \
        addwz_ri(Reax, IB(RT_SIMD_MASK_##mask##32_128))                     \
        jezxx_lb(lb)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
 * original FCTRL blocks (FCTRL_ENTER/FCTRL_LEAVE) are defined in rtbase.h
 * NOTE: ARMv7 always uses ROUNDN non-IEEE mode for SIMD fp-arithmetic,
 * while fp<->int conversion takes ROUND* into account via VFP fallback */

#if RT_SIMD_FLUSH_ZERO == 0

#define RT_SIMD_MODE_ROUNDN     0x00    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x02    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x01    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#else /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN     0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM     0x06    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP     0x05    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ     0x07    /* round towards zero */

#endif /* RT_SIMD_FLUSH_ZERO */

#define RT_SIMD_MODE_ROUNDN_F   0x04    /* round towards near */
#define RT_SIMD_MODE_ROUNDM_F   0x06    /* round towards -inf */
#define RT_SIMD_MODE_ROUNDP_F   0x05    /* round towards +inf */
#define RT_SIMD_MODE_ROUNDZ_F   0x07    /* round towards zero */

#define fpscr_ld(RS) /* not portable, do not use outside */                 \
        EMITW(0xD51B4400 | MRM(REG(RS), 0x00,    0x00))

#define fpscr_st(RD) /* not portable, do not use outside */                 \
        EMITW(0xD53B4400 | MRM(REG(RD), 0x00,    0x00))

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0x52A00000 | MRM(TIxx,    0x00,    0x00) |                    \
                           RT_SIMD_MODE_##mode << 11)                       \
        EMITW(0xD51B4400 | MRM(TIxx,    0x00,    0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xD51B4400 | MRM(TNxx,    0x00,    0x00))

#else /* RT_SIMD_FAST_FCTRL */

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        EMITW(0xD51B4400 | MRM(TNxx+(RT_SIMD_MODE_##mode&3), 0x00, 0x00))

#define FCTRL_RESET()     /* resumes default mode (ROUNDN) upon leave */    \
        EMITW(0xD51B4400 | MRM(TNxx,    0x00,    0x00))

#endif /* RT_SIMD_FAST_FCTRL */

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndis_rr(XD, XS)                                                    \
        EMITW(0x6EA19800 | MXM(REG(XD), REG(XS), 0x00))

#define rndis_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x6EA19800 | MXM(REG(XD), TmmM,    0x00))

#define cvtis_rr(XD, XS)                                                    \
        rndis_rr(W(XD), W(XS))                                              \
        cvzis_rr(W(XD), W(XD))

#define cvtis_ld(XD, MS, DS)                                                \
        rndis_ld(W(XD), W(MS), W(DS))                                       \
        cvzis_rr(W(XD), W(XD))

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvtin_rr(XD, XS)                                                    \
        EMITW(0x4E21D800 | MXM(REG(XD), REG(XS), 0x00))

#define cvtin_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x3DC00000 | MPM(TmmM,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x4E21D800 | MXM(REG(XD), TmmM,    0x00))

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnris_rr(XD, XS, mode)                                              \
        EMITW(0x4E218800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

#define cvris_rr(XD, XS, mode)                                              \
        EMITW(0x4E21A800 | MXM(REG(XD), REG(XS), 0x00) |                    \
        (RT_SIMD_MODE_##mode&1) << 23 | (RT_SIMD_MODE_##mode&2) << 11)

/**************   scalar single precision floating point (NEON)   *************/

/* mov (D = S) */

#define movrx_rr(XD, XS)                                                    \
        EMITW(0x5E040400 | MXM(REG(XD), REG(XS), 0x00))

#define movrx_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(REG(XD), TPxx,    0x00))

#define movrx_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MD), VSL(DD), B1(DD), P1(DD)))  \
        EMITW(0x0D008000 | MXM(REG(XS), TPxx,    0x00))

/* add (G = G + S) */

#define addrs_rr(XG, XS)                                                    \
        EMITW(0x1E202800 | MXM(REG(XG), REG(XG), REG(XS)))

#define addrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1E202800 | MXM(REG(XG), REG(XG), TmmM))

/* sub (G = G - S) */

#define subrs_rr(XG, XS)                                                    \
        EMITW(0x1E203800 | MXM(REG(XG), REG(XG), REG(XS)))

#define subrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1E203800 | MXM(REG(XG), REG(XG), TmmM))

/* mul (G = G * S) */

#define mulrs_rr(XG, XS)                                                    \
        EMITW(0x1E200800 | MXM(REG(XG), REG(XG), REG(XS)))

#define mulrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1E200800 | MXM(REG(XG), REG(XG), TmmM))

/* div (G = G / S) */

#define divrs_rr(XG, XS)                                                    \
        EMITW(0x1E201800 | MXM(REG(XG), REG(XG), REG(XS)))

#define divrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1E201800 | MXM(REG(XG), REG(XG), TmmM))

/* sqr (D = sqrt S) */

#define sqrrs_rr(XD, XS)                                                    \
        EMITW(0x1E21C000 | MXM(REG(XD), REG(XS), 0x00))

#define sqrrs_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1E21C000 | MXM(REG(XD), TmmM,    0x00))

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rcers_rr(XD, XS)                                                    \
        EMITW(0x5EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rcsrs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x5E20FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1E200800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rsers_rr(XD, XS)                                                    \
        EMITW(0x7EA1D800 | MXM(REG(XD), REG(XS), 0x00))

#define rssrs_rr(XG, XS) /* destroys XS */                                  \
        EMITW(0x1E200800 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x5EA0FC00 | MXM(REG(XS), REG(XS), REG(XG)))                  \
        EMITW(0x1E200800 | MXM(REG(XG), REG(XG), REG(XS)))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmars_rr(XG, XS, XT)                                                \
        EMITW(0x1F000000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmars_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MT), VSL(DT), B1(DT), P1(DT)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1F000000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsrs_rr(XG, XS, XT)                                                \
        EMITW(0x1F008000 | MXM(REG(XG), REG(XS), REG(XT)) | REG(XG) << 10)

#define fmsrs_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MT), VSL(DT), B1(DT), P1(DT)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1F008000 | MXM(REG(XG), REG(XS), TmmM) | REG(XG) << 10)

#endif /* RT_SIMD_COMPAT_FMS */

/* min (G = G < S ? G : S) */

#define minrs_rr(XG, XS)                                                    \
        EMITW(0x1E205800 | MXM(REG(XG), REG(XG), REG(XS)))

#define minrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1E205800 | MXM(REG(XG), REG(XG), TmmM))

/* max (G = G > S ? G : S) */

#define maxrs_rr(XG, XS)                                                    \
        EMITW(0x1E204800 | MXM(REG(XG), REG(XG), REG(XS)))

#define maxrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x1E204800 | MXM(REG(XG), REG(XG), TmmM))

/* cmp (G = G ? S) */

#define ceqrs_rr(XG, XS)                                                    \
        EMITW(0x5E20E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x5E20E400 | MXM(REG(XG), REG(XG), TmmM))

#define cners_rr(XG, XS)                                                    \
        EMITW(0x5E20E400 | MXM(REG(XG), REG(XG), REG(XS)))                  \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

#define cners_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x5E20E400 | MXM(REG(XG), REG(XG), TmmM))                     \
        EMITW(0x6E205800 | MXM(REG(XG), REG(XG), 0x00))

#define cltrs_rr(XG, XS)                                                    \
        EMITW(0x7EA0E400 | MXM(REG(XG), REG(XS), REG(XG)))

#define cltrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x7EA0E400 | MXM(REG(XG), TmmM,    REG(XG)))

#define clers_rr(XG, XS)                                                    \
        EMITW(0x7E20E400 | MXM(REG(XG), REG(XS), REG(XG)))

#define clers_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x7E20E400 | MXM(REG(XG), TmmM,    REG(XG)))

#define cgtrs_rr(XG, XS)                                                    \
        EMITW(0x7EA0E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgtrs_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x7EA0E400 | MXM(REG(XG), REG(XG), TmmM))

#define cgers_rr(XG, XS)                                                    \
        EMITW(0x7E20E400 | MXM(REG(XG), REG(XG), REG(XS)))

#define cgers_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x91000000 | MDM(TPxx,    MOD(MS), VSL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0D408000 | MXM(TmmM,    TPxx,    0x00))                     \
        EMITW(0x7E20E400 | MXM(REG(XG), REG(XG), TmmM))

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
        EMITW(0x3D800000 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x3D800000 | MXM(TmmF,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x3D800000 | MXM(TmmM,    Teax,    0x00))

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
        EMITW(0x3DC00000 | MXM(TmmE,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x3DC00000 | MXM(TmmF,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32_128*4))                           \
        EMITW(0x3DC00000 | MXM(TmmM,    Teax,    0x00))

#ifndef RT_RTARCH_A64_256V1_H
#undef  RT_256
#define RT_256  (RT_128)
#include "rtarch_a64_256v1.h"
#endif /* RT_RTARCH_A64_256V1_H */

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A32_128V1_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

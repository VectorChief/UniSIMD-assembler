/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_M32_128_H
#define RT_RTARCH_M32_128_H

#include "rtarch_m64.h"

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       16
#define RT_SIMD_WIDTH32     4
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH64     2
#define RT_SIMD_SET64(s, v) s[0]=s[1]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la
#undef  movox_ld
#undef  movqx_ld
#define movqx_ld(XD, MS, DS)
#undef  EMITS
#define EMITS(w) EMITW(w)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_m32_128.h: Implementation of MIPS fp32 MSA instructions.
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
 * cmdpx_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmdpn_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmdps_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * cmdo*_** - applies [cmd] to 32-bit SIMD register/memory/immediate args
 * cmdp*_** - applies [cmd] to L-size SIMD register/memory/immediate args
 * cmdq*_** - applies [cmd] to 64-bit SIMD register/memory/immediate args
 *
 * The cmdp*_** instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
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

#define TmmS    0x1C  /* w28, sign-mask 32-bit */
#define TmmT    0x1D  /* w29, sign-mask 64-bit */
#define TmmZ    0x1E  /* w30, zero-mask all 0s */
#define Tmm1    0x1F  /* w31 */

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
#define XmmF    0x0F, $w15, EMPTY       /* w15 */

/******************************************************************************/
/**********************************   MSA   ***********************************/
/******************************************************************************/

/**************************   packed generic (SIMD)   *************************/

/* mov */

#define movox_rr(XD, XS)                                                    \
        EMITW(0x78BE0019 | MXM(REG(XD), REG(XS), 0x00))

#define movox_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(REG(XD), MOD(MS), VAL(DS), B2(DS), P2(DS)))

#define movox_st(XS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C2(DD), EMPTY2)   \
        EMITW(0x78000027 | MPM(REG(XS), MOD(MD), VAL(DD), B2(DD), P2(DD)))


#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, DS is SIMD-aligned */     \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C3(DS), EMPTY2)   \
        EMITW(0x00000021 | MRM(REG(RD), MOD(MS), TDxx) | ADR)

/* and */

#define andox_rr(XG, XS)                                                    \
        EMITW(0x7800001E | MXM(REG(XG), REG(XG), REG(XS)))

#define andox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7800001E | MXM(REG(XG), REG(XG), Tmm1))

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        EMITW(0x78C0001E | MXM(REG(XG), REG(XS), TmmZ))

#define annox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001E | MXM(REG(XG), Tmm1,    TmmZ))

/* orr */

#define orrox_rr(XG, XS)                                                    \
        EMITW(0x7820001E | MXM(REG(XG), REG(XG), REG(XS)))

#define orrox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7820001E | MXM(REG(XG), REG(XG), Tmm1))

/* orn (G = ~G | S) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

/* xor */

#define xorox_rr(XG, XS)                                                    \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), REG(XS)))

#define xorox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), Tmm1))

/* not */

#define notox_rx(XG)                                                        \
        EMITW(0x7840001E | MXM(REG(XG), TmmZ,    REG(XG)))

/**************   packed single precision floating point (SIMD)   *************/

/* neg */

#define negos_rx(XG)                                                        \
        EMITW(0x7860001E | MXM(REG(XG), REG(XG), TmmS))

/* fma (G = G + S * T) */

#define fmaos_rr(XG, XS, XT)                                                \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7900001B | MXM(REG(XG), REG(XS), Tmm1))

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), REG(XT)))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C2(DT), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MT), VAL(DT), B2(DT), P2(DT)))  \
        EMITW(0x7940001B | MXM(REG(XG), REG(XS), Tmm1))

/* add */

#define addos_rr(XG, XS)                                                    \
        EMITW(0x7800001B | MXM(REG(XG), REG(XG), REG(XS)))

#define addos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7800001B | MXM(REG(XG), REG(XG), Tmm1))

/* sub */

#define subos_rr(XG, XS)                                                    \
        EMITW(0x7840001B | MXM(REG(XG), REG(XG), REG(XS)))

#define subos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7840001B | MXM(REG(XG), REG(XG), Tmm1))

/* mul */

#define mulos_rr(XG, XS)                                                    \
        EMITW(0x7880001B | MXM(REG(XG), REG(XG), REG(XS)))

#define mulos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7880001B | MXM(REG(XG), REG(XG), Tmm1))

/* div */

#define divos_rr(XG, XS)                                                    \
        EMITW(0x78C0001B | MXM(REG(XG), REG(XG), REG(XS)))

#define divos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001B | MXM(REG(XG), REG(XG), Tmm1))

/* sqr */

#define sqros_rr(XD, XS)                                                    \
        EMITW(0x7B26001E | MXM(REG(XD), REG(XS), 0x00))

#define sqros_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B26001E | MXM(REG(XD), Tmm1,    0x00))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XD, XS)                                                    \
        EMITW(0x7B2A001E | MXM(REG(XD), REG(XS), 0x00))

#define rcsos_rr(XG, XS) /* destroys MS */

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseos_rr(XD, XS)                                                    \
        EMITW(0x7B28001E | MXM(REG(XD), REG(XS), 0x00))

#define rssos_rr(XG, XS) /* destroys MS */

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minos_rr(XG, XS)                                                    \
        EMITW(0x7B00001B | MXM(REG(XG), REG(XG), REG(XS)))

#define minos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B00001B | MXM(REG(XG), REG(XG), Tmm1))

/* max */

#define maxos_rr(XG, XS)                                                    \
        EMITW(0x7B80001B | MXM(REG(XG), REG(XG), REG(XS)))

#define maxos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B80001B | MXM(REG(XG), REG(XG), Tmm1))

/* cmp */

#define ceqos_rr(XG, XS)                                                    \
        EMITW(0x7880001A | MXM(REG(XG), REG(XG), REG(XS)))

#define ceqos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7880001A | MXM(REG(XG), REG(XG), Tmm1))

#define cneos_rr(XG, XS)                                                    \
        EMITW(0x78C0001C | MXM(REG(XG), REG(XG), REG(XS)))

#define cneos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0001C | MXM(REG(XG), REG(XG), Tmm1))

#define cltos_rr(XG, XS)                                                    \
        EMITW(0x7900001A | MXM(REG(XG), REG(XG), REG(XS)))

#define cltos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7900001A | MXM(REG(XG), REG(XG), Tmm1))

#define cleos_rr(XG, XS)                                                    \
        EMITW(0x7980001A | MXM(REG(XG), REG(XG), REG(XS)))

#define cleos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7980001A | MXM(REG(XG), REG(XG), Tmm1))

#define cgtos_rr(XG, XS)                                                    \
        EMITW(0x7900001A | MXM(REG(XG), REG(XS), REG(XG)))

#define cgtos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7900001A | MXM(REG(XG), Tmm1,    REG(XG)))

#define cgeos_rr(XG, XS)                                                    \
        EMITW(0x7980001A | MXM(REG(XG), REG(XS), REG(XG)))

#define cgeos_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7980001A | MXM(REG(XG), Tmm1,    REG(XG)))

/**************************   packed integer (SIMD)   *************************/

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        cvzos_rr(W(XD), W(XS))                                              \
        cvnon_rr(W(XD), W(XD))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
        cvzos_ld(W(XD), W(MS), W(DS))                                       \
        cvnon_rr(W(XD), W(XD))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        EMITW(0x7B22001E | MXM(REG(XD), REG(XS), 0x00))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B22001E | MXM(REG(XD), Tmm1,    0x00))

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDP)

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        rndos_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_ld(W(XD), W(MS), W(DS))                                       \
        FCTRL_LEAVE(ROUNDM)

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        rndos_rr(W(XD), W(XS))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
        rndos_ld(W(XD), W(MS), W(DS))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        cvtos_rr(W(XD), W(XS))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        cvtos_ld(W(XD), W(MS), W(DS))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

/* add */

#define addox_rr(XG, XS)                                                    \
        EMITW(0x7840000E | MXM(REG(XG), REG(XG), REG(XS)))

#define addox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7840000E | MXM(REG(XG), REG(XG), Tmm1))

/* sub */

#define subox_rr(XG, XS)                                                    \
        EMITW(0x78C0000E | MXM(REG(XG), REG(XG), REG(XS)))

#define subox_ld(XG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x78C0000E | MXM(REG(XG), REG(XG), Tmm1))

/* shl */

#define shlox_ri(XG, IS)                                                    \
        EMITW(0x78400009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B02001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x7840000D | MXM(REG(XG), REG(XG), Tmm1))

/* shr */

#define shrox_ri(XG, IS)                                                    \
        EMITW(0x79400009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B02001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x7940000D | MXM(REG(XG), REG(XG), Tmm1))

#define shron_ri(XG, IS)                                                    \
        EMITW(0x78C00009 | MXM(REG(XG), REG(XG), 0x00) |                    \
                                                 (0x1F & VAL(IS)) << 16)

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x8C000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x7B02001E | MXM(Tmm1,    TMxx,    0x00))                     \
        EMITW(0x78C0000D | MXM(REG(XG), REG(XG), Tmm1))

/**************************   helper macros (SIMD)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       MN      /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       MF      /*  all satisfy the condition */

#define S0(mask)    S1(mask)
#define S1(mask)    S##mask
#define SMN(rg, lb) ASM_BEG ASM_OP2( bz.v, rg, lb) ASM_END
#define SMF(rg, lb) ASM_BEG ASM_OP2(bnz.w, rg, lb) ASM_END

#define CHECK_MASK(lb, mask, XS) /* destroys Reax */                        \
        AUW(EMPTY, EMPTY, EMPTY, MOD(XS), lb, S0(RT_SIMD_MASK_##mask), EMPTY2)

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

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        EMITW(0x7B2C001E | MXM(REG(XD), REG(XS), 0x00))

#define rndos_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B2C001E | MXM(REG(XD), Tmm1,    0x00))

#define cvtos_rr(XD, XS)                                                    \
        EMITW(0x7B38001E | MXM(REG(XD), REG(XS), 0x00))

#define cvtos_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B38001E | MXM(REG(XD), Tmm1,    0x00))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        EMITW(0x7B3C001E | MXM(REG(XD), REG(XS), 0x00))

#define cvton_ld(XD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C2(DS), EMPTY2)   \
        EMITW(0x78000023 | MPM(Tmm1,    MOD(MS), VAL(DS), B2(DS), P2(DS)))  \
        EMITW(0x7B3C001E | MXM(REG(XD), Tmm1,    0x00))

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        rndos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

#define cvros_rr(XD, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtos_rr(W(XD), W(XS))                                              \
        FCTRL_LEAVE(mode)

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
        EMITW(0x78000027 | MXM(TmmZ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x78000027 | MXM(Tmm1,    Teax,    0x00))

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
        EMITW(0x78000023 | MXM(TmmZ,    Teax,    0x00))                     \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
        EMITW(0x78000023 | MXM(Tmm1,    Teax,    0x00))

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_M32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

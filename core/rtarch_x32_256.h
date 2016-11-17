/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_256_H
#define RT_RTARCH_X32_256_H

#include "rtarch_x64.h"

#if defined (RT_256) && (RT_256 != 0)

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       32
#define RT_SIMD_WIDTH32     8
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v
#define RT_SIMD_WIDTH64     4
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=v

#endif /* RT_256 */

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_256.h: Implementation of x86_64 fp32 AVX(1,2) instructions.
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
 * The cmdp*_** (rtbase.h) instructions are intended for SPMD programming model
 * and can be configured to work with 32/64-bit data-elements (int, fp).
 * In this model data-paths are fixed-width, BASE and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 * Matching element-sized BASE subset cmdy*_** is defined in rtbase.h.
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

#if defined (RT_256) && (RT_256 != 0) || defined (RT_128) && (RT_128 >= 8)

#undef  sregs_sa
#undef  sregs_la
#undef  mxcsr_ld

#if   defined (RT_256) && (RT_256 != 0)

#undef RT_128 /* turn off 128-bit definitions in multi-target build */

#define K 1

#elif defined (RT_128) && (RT_128 >= 8)

#define K 0

#endif /* RT_256, RT_128 */

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
#define Xmm8    0x08, 0x03, EMPTY
#define Xmm9    0x09, 0x03, EMPTY
#define XmmA    0x0A, 0x03, EMPTY
#define XmmB    0x0B, 0x03, EMPTY
#define XmmC    0x0C, 0x03, EMPTY
#define XmmD    0x0D, 0x03, EMPTY
#if     RT_SIMD_COMPAT_XMM < 2
#define XmmE    0x0E, 0x03, EMPTY            /* may be reserved in some cases */
#if     RT_SIMD_COMPAT_XMM < 1
#define XmmF    0x0F, 0x03, EMPTY            /* may be reserved in some cases */
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
/**********************************   AVX   ***********************************/
/******************************************************************************/

/* adr (D = adr S) */

#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, MS/DS is SIMD-aligned */  \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x8D)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/**************************   packed generic (AVX1)   *************************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movox_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movox_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvox_rr(XG, XS)                                                    \
    ADR VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define mmvox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define mmvox_st(XS, MG, DG)                                                \
    ADR VEX(RXB(XS), RXB(MG),    0x00, K, 1, 2) EMITB(0x2E)                 \
        MRM(REG(XS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* and (G = G & S) */

#define andox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orr (G = G | S) */

#define orrox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orn (G = ~G | S) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        annox_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (AVX1)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        xorox_ld(W(XG), Mebp, inf_GPC06_32)

/* add (G = G + S) */

#define addos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub (G = G - S) */

#define subos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* mul (G = G * S) */

#define mulos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* div (G = G / S) */

#define divos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqros_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x53)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys XS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        addos_rr(W(XG), W(XG))                                              \
        subos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq (D = 1.0 / sqrt S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ != 1

#define rseos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x52)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys XS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        subos_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulos_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if defined (RT_256) && (RT_256 < 2) || \
    defined (RT_128) && (RT_SIMD_COMPAT_128 == 1)

#define addzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x00,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x05,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))


#define cvycs_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvycs_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cvxds_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XD), RXB(XS),    0x00, 1, 1, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))


#define addds_rr(XG, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subds_rr(XG, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulds_rr(XG, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(XS), REG(XS))


#define addds_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subds_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mulds_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_rr(W(XS), W(XT))                                              \
        addos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_ld(W(XS), W(MT), W(DT))                                       \
        addos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMA == 1

#if defined (RT_256) && (RT_256 < 2) /* NOTE: 2-pass fp32<->fp64 SIMD FMA */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvycs_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        subzm_ri(W(MT), IC(0x10))                  /* 2st-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#else /* RT_128 >= 8 */ /* NOTE: 1-pass fp32<->fp64 SIMD FMA (128-bit AVX1) */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movox_ld(W(XS), Mebp, inf_SCR02(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XS), W(MT), W(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        addds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movox_ld(W(XS), Mebp, inf_SCR02(0))

#endif /* RT_128 >= 8 */

#endif /* RT_SIMD_COMPAT_FMA */

#if RT_SIMD_COMPAT_FMS == 0

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_rr(W(XS), W(XT))                                              \
        subos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        mulos_ld(W(XS), W(MT), W(DT))                                       \
        subos_rr(W(XG), W(XS))                                              \
        movox_ld(W(XS), Mebp, inf_SCR01(0))

#elif RT_SIMD_COMPAT_FMS == 1

#if defined (RT_256) && (RT_256 < 2) /* NOTE: 2-pass fp32<->fp64 SIMD FMS */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvycs_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvycs_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XG), W(XS))                                              \
        mulds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subds_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxds_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        subzm_ri(W(MT), IC(0x10))                  /* 2st-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#else /* RT_128 >= 8 */ /* NOTE: 1-pass fp32<->fp64 SIMD FMS (128-bit AVX1) */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        cvycs_rr(W(XS), W(XT))                                              \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movox_ld(W(XS), Mebp, inf_SCR02(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvycs_rr(W(XG), W(XS))                     /* 1st-pass -> */        \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        cvycs_ld(W(XS), W(MT), W(DT))                                       \
        mulds_rr(W(XS), W(XG))                                              \
        cvycs_ld(W(XG), Mebp, inf_SCR01(0))                                 \
        subds_rr(W(XG), W(XS))                                              \
        cvxds_rr(W(XG), W(XG))                     /* 1st-pass <- */        \
        movox_ld(W(XS), Mebp, inf_SCR02(0))

#endif /* RT_128 >= 8 */

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_256 >= 2 */ /* NOTE: FMA is available in processors with AVX2 */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
    ADR VEX(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
    ADR VEX(RXB(XG), RXB(XT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(RXB(XG), RXB(MT), REN(XS), K, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_256 >= 2 */

/* min (G = G < S ? G : S) */

#define minos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* max (G = G > S ? G : S) */

#define maxos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cmp (G = G ? S) */

#define ceqos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cneos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cltos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cleos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cgtos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgeos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvm (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cvnos_rr(XD, XS)     /* round towards near */                       \
        cvtos_rr(W(XD), W(XS))

#define cvnos_ld(XD, MS, DS) /* round towards near */                       \
        cvtos_ld(W(XD), W(MS), W(DS))

/* cvn (D = signed-int-to-fp S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XD, XS)     /* round towards near */                       \
        cvton_rr(W(XD), W(XS))

#define cvnon_ld(XD, MS, DS) /* round towards near */                       \
        cvton_ld(W(XD), W(MS), W(DS))

/**************************   extended float (x87)   **************************/

#define fpuws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD9)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuws_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xD9)                                   \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpuwn_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDB)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuwn_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDB)                                   \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#define fpuwt_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDB)                                   \
        MRM(0x01,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define addws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbrws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x01,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define dvrws_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD8)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define addes_xn(n)      /* ST(0) = ST(0) + ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xC0+(n))

#define addes_nx(n)      /* ST(n) = ST(n) + ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xC0+(n))

#define addes_np(n)      /* ST(n) = ST(n) + ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xC0+(n))


#define subes_xn(n)      /* ST(0) = ST(0) - ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xE0+(n))

#define subes_nx(n)      /* ST(n) = ST(n) - ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xE8+(n))

#define subes_np(n)      /* ST(n) = ST(n) - ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xE8+(n))


#define sbres_xn(n)      /* ST(0) = ST(n) - ST(0), don't pop */             \
        EMITB(0xD8) EMITB(0xE8+(n))

#define sbres_nx(n)      /* ST(n) = ST(0) - ST(n), don't pop */             \
        EMITB(0xDC) EMITB(0xE0+(n))

#define sbres_np(n)      /* ST(n) = ST(0) - ST(n), pop stack */             \
        EMITB(0xDE) EMITB(0xE0+(n))


#define mules_xn(n)      /* ST(0) = ST(0) * ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xC8+(n))

#define mules_nx(n)      /* ST(n) = ST(n) * ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xC8+(n))

#define mules_np(n)      /* ST(n) = ST(n) * ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xC8+(n))


#define dives_xn(n)      /* ST(0) = ST(0) / ST(n), don't pop */             \
        EMITB(0xD8) EMITB(0xF0+(n))

#define dives_nx(n)      /* ST(n) = ST(n) / ST(0), don't pop */             \
        EMITB(0xDC) EMITB(0xF8+(n))

#define dives_np(n)      /* ST(n) = ST(n) / ST(0), pop stack */             \
        EMITB(0xDE) EMITB(0xF8+(n))


#define dvres_xn(n)      /* ST(0) = ST(n) / ST(0), don't pop */             \
        EMITB(0xD8) EMITB(0xF8+(n))

#define dvres_nx(n)      /* ST(n) = ST(0) / ST(n), don't pop */             \
        EMITB(0xDC) EMITB(0xF0+(n))

#define dvres_np(n)      /* ST(n) = ST(0) / ST(n), pop stack */             \
        EMITB(0xDE) EMITB(0xF0+(n))


#define sqres_xx()       /* ST(0) = sqr ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xFA)

#define neges_xx()       /* ST(0) = neg ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xE0)

#define abses_xx()       /* ST(0) = abs ST(0), don't pop */                 \
        EMITB(0xD9) EMITB(0xE1)


#define xm2es_xx()       /* ST(0) = 2^ST(0)-1, don't pop, [-1.0 : +1.0] */  \
        EMITB(0xD9) EMITB(0xF0)

#define lg2es_xx()       /* ST(1) = ST(1)*lg2 ST(0), pop stack */           \
        EMITB(0xD9) EMITB(0xF1)

#define lp2es_xx()       /* ST(1) = ST(1)*lg2 ST(0)+1.0, pop stack */       \
        EMITB(0xD9) EMITB(0xF9)


#define sines_xx()       /* ST(0) = sin ST(0), don't pop, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xFE)

#define coses_xx()       /* ST(0) = cos ST(0), don't pop, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xFF)

#define scses_xx()       /* ST(0) = sin ST(0), push cos ST(0), original */  \
        EMITB(0xD9) EMITB(0xFB)

#define tanes_xx()       /* ST(0) = tan ST(0), push +1.0, [-2^63:+2^63] */  \
        EMITB(0xD9) EMITB(0xF2)

#define atnes_xx()       /* ST(1) = atn ST(1)/ST(0), pop stack */           \
        EMITB(0xD9) EMITB(0xF3)


#define remes_xx()       /* ST(0) = ST(0)-Q*ST(1), Q = rnd ST(0)/ST(1) */   \
        EMITB(0xD9) EMITB(0xF5)

#define rexes_xx()       /* ST(0) = ST(0)-Q*ST(1), Q = trn ST(0)/ST(1) */   \
        EMITB(0xD9) EMITB(0xF8)

#define rndes_xx()       /* ST(0) = rnd ST(0), round to integral value */   \
        EMITB(0xD9) EMITB(0xFC)

#define extes_xx()       /* ST(0) = exp ST(0), push mts ST(0) */            \
        EMITB(0xD9) EMITB(0xF4)

#define scles_xx()       /* exp ST(0) = exp ST(0) + rnd ST(1) */            \
        EMITB(0xD9) EMITB(0xFD)


#define cmpes_xn(n)      /* flags = ST(0) ? ST(n), don't pop */             \
        EMITB(0xDB) EMITB(0xF0+(n))

#define cmpes_pn(n)      /* flags = ST(0) ? ST(n), pop stack */             \
        EMITB(0xDF) EMITB(0xF0+(n))

#define moves_nx(n)      /* ST(n) = ST(0), don't pop */                     \
        EMITB(0xDD) EMITB(0xD0+(n))

#define moves_np(n)      /* ST(n) = ST(0), pop stack */                     \
        EMITB(0xDD) EMITB(0xD8+(n))

#define popes_xx()       /* ST(0) = ST(0), pop stack */                     \
        EMITB(0xDD) EMITB(0xD8)


#define fpucw_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xD9)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpucw_st(MD, DD) /* not portable, do not use outside */             \
FWT ADR REX(0,       RXB(MD)) EMITB(0xD9)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpurz_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR02(4))

/**************************   packed integer (AVX1)   *************************/

#if defined (RT_256) && (RT_256 < 2)

#define prmox_rr(XD, XS, IT) /* not portable, do not use outside */         \
        VEX(RXB(XD), RXB(XS), REN(XD), K, 1, 3) EMITB(0x06)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define movix_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XD), RXB(MS),    0x00, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movix_st(XS, MD, DD) /* not portable, do not use outside */         \
    ADR VEX(RXB(XS), RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add (G = G + S) */

#define addix_rr(XG, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 0, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        addix_rr(W(XG), W(XS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addix_rr(W(XG), W(XS))                                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define addix_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addox_ld(XG, MS, DS)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* sub (G = G - S) */

#define subix_rr(XG, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XG), RXB(XS), REN(XG), 0, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        subix_rr(W(XG), W(XS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        subix_rr(W(XG), W(XS))                                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define subix_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subox_ld(XG, MS, DS)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlix_ri(XG, IS)     /* not portable, do not use outside */         \
        VEX(0,       RXB(XG), REN(XG), 0, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlox_ri(XG, IS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shlix_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrix_ri(XG, IS)     /* not portable, do not use outside */         \
        VEX(0,       RXB(XG), REN(XG), 0, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrox_ri(XG, IS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shrix_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))


#define shrin_ri(XG, IS)     /* not portable, do not use outside */         \
        VEX(0,       RXB(XG), REN(XG), 0, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shron_ri(XG, IS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shrin_ld(XG, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 0, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x00))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x04))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x08))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x0C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x10))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x10))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x14))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x14))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x18))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x18))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x1C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x1C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_128 >= 8 || RT_256 >= 2 */

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#if defined (RT_128) && (RT_SIMD_COMPAT_128 == 1)

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#else /* (RT_128 && RT_SIMD_COMPAT_128 == 2) || RT_256 */

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* (RT_128 && RT_SIMD_COMPAT_128 == 2) || RT_256 */

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#if defined (RT_128) && (RT_SIMD_COMPAT_128 == 1)

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#else /* (RT_128 && RT_SIMD_COMPAT_128 == 2) || RT_256 */

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* (RT_128 && RT_SIMD_COMPAT_128 == 2) || RT_256 */


#define shron_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), K, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#if defined (RT_128) && (RT_SIMD_COMPAT_128 == 1)

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(MS), W(DS))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#else /* (RT_128 && RT_SIMD_COMPAT_128 == 2) || RT_256 */

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(RXB(XG), RXB(XS), REN(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(RXB(XG), RXB(MS), REN(XG), K, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* (RT_128 && RT_SIMD_COMPAT_128 == 2) || RT_256 */

#endif /* RT_128 >= 8 || RT_256 >= 2 */

/**************************   helper macros (AVX1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00        /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x0F+K*0xF0 /*  all satisfy the condition */

#define movsn_rr(RD, XS) /* not portable, do not use outside */             \
        VEX(RXB(RD), RXB(XS),    0x00, K, 0, 1) EMITB(0x50)                 \
        MRM(REG(RD), MOD(XS), REG(XS))

#define CHECK_MASK(lb, mask, XS) /* destroys Reax, jump lb if mask == S */  \
        movsn_rr(Reax, W(XS))                                               \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask))                             \
        jeqxx_lb(lb)

/* simd mode
 * set via FCTRL macros, *_F for faster non-IEEE mode (optional on MIPS/Power),
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
    ADR VEX(0,       RXB(MS),    0x00, 0, 0, 1) EMITB(0xAE)                 \
        MRM(0x02,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mxcsr_st(MD, DD) /* not portable, do not use outside */             \
    ADR VEX(0,       RXB(MD),    0x00, 0, 0, 1) EMITB(0xAE)                 \
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

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtos_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvton_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),    0x00, K, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        VEX(RXB(XD), RXB(XS),    0x00, K, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)                                        \
        cvzos_rr(W(XD), W(XD))

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
    ADR VEX(1,             0,    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(0x06,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
    ADR VEX(1,             0,    0x00, K, 0, 1) EMITB(0x29)                 \
        MRM(0x07,       0x00,    0x00)

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
    ADR VEX(1,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(0x06,       0x00,    0x00)                                      \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH32*4))                               \
    ADR VEX(1,             0,    0x00, K, 0, 1) EMITB(0x28)                 \
        MRM(0x07,       0x00,    0x00)

#endif /* RT_256, RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

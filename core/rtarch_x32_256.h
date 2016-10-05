/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_256_H
#define RT_RTARCH_X32_256_H

#include "rtarch_x64.h"

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       32
#define RT_SIMD_WIDTH32     8
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v
#define RT_SIMD_WIDTH64     4
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la
#undef  mxcsr_ld

#if defined (RT_256) && (RT_256 != 0)

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

/* 3-byte VEX prefix with full customization (W0) */
#define VEX(rxg, rxm, ren, len, pfx, aux)                                   \
        EMITB(0xC4)                                                         \
        EMITB((1 - (rxg)) << 7 | 1 << 6 | (1 - (rxm)) << 5 | (aux))         \
        EMITB((len) << 2 | (0x0F - (ren)) << 3 | (pfx))

/* selector for full register (3rd operand, 4-bits-wide) */
#define REN(reg, mod, sib)  reg

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
#define XmmE    0x0E, 0x03, EMPTY
#define XmmF    0x0F, 0x03, EMPTY

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov */

#define movox_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movox_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movox_st(XS, MD, DD)                                                \
    ADR VEX(RXB(XS), RXB(MD),     0x0, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define adrpx_ld(RD, MS, DS) /* RD is a BASE reg, DS is SIMD-aligned */     \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x8D)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* and */

#define andox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orr */

#define orrox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* orn (G = ~G | S) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

/* xor */

#define xorox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* not */

#define notox_rx(XG)                                                        \
        annox_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (AVX1)   *************/

/* neg */

#define negos_rx(XG)                                                        \
        xorox_ld(W(XG), Mebp, inf_GPC06_32)

/* add */

#define addos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub */

#define subos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* mul */

#define mulos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* div */

#define divos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sqr */

#define sqros_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqros_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 0, 1) EMITB(0x53)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys MS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        addos_rr(W(XG), W(XG))                                              \
        subos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 0, 1) EMITB(0x52)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys MS */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        subos_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulos_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

#if (RT_256 < 2) /* NOTE: implement 2-pass fp32<->fp64 FMA fallback below */

#define cvqos_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvqos_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cvoqs_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define addzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x00,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x05,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#if RT_SIMD_COMPAT_FMA == 0

/* fma (G = G + S * T) */

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

#else /* RT_SIMD_COMPAT_FMA */

/* fma (G = G + S * T) */

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvqos_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvqos_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvqos_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvqos_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        subzm_ri(W(MT), IC(0x10))                  /* 2st-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

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

#else /* RT_SIMD_COMPAT_FMS */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvqos_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvqos_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvqos_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvqos_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvqos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvoqs_rr(W(XG), W(XG))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        subzm_ri(W(MT), IC(0x10))                  /* 2st-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_256 >= 2 */ /* NOTE: FMA is available in processors with AVX2 */

/* fma (G = G + S * T) */

#define fmaos_rr(XG, XS, XT)                                                \
    ADR VEX(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
    ADR VEX(RXB(XG), RXB(XT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(RXB(XG), RXB(MT), REN(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#endif /* RT_256 >= 2 */

/* min */

#define minos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* max */

#define maxos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cmp */

#define ceqos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

#define cneos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cltos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cleos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cgtos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x06))

#define cgeos_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeos_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x05))

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x03))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x02))

#define cvpos_rr(XD, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvpos_ld(XD, MS, DS) /* round towards +inf */                       \
        rnpos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XD, XS)     /* round towards -inf */                       \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x01))

#define cvmos_rr(XD, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XD), W(XS))                                              \
        cvzos_rr(W(XD), W(XD))

#define cvmos_ld(XD, MS, DS) /* round towards -inf */                       \
        rnmos_ld(W(XD), W(MS), W(DS))                                       \
        cvzos_rr(W(XD), W(XD))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XD, XS)     /* round towards near */                       \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x00))

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

#if (RT_256 < 2)

#define prmox_rr(XD, XS, IT) /* not portable, do not use outside */         \
        VEX(RXB(XD), RXB(XS), REN(XD), 1, 1, 3) EMITB(0x06)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

#define movix_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 0, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movix_st(XS, MD, DD) /* not portable, do not use outside */         \
    ADR VEX(RXB(XS), RXB(MD),     0x0, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* add */

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

/* sub */

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

/* shl */

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

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shr */

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

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
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

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_256 >= 2 */

/* add */

#define addox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* sub */

#define subox_rr(XG, XS)                                                    \
        VEX(RXB(XG), RXB(XS), REN(XG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_ld(XG, MS, DS)                                                \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shl */

#define shlox_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* shr */

#define shrox_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define shron_ri(XG, IS)                                                    \
        VEX(0,       RXB(XG), REN(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 1 elem at given address */ \
    ADR VEX(RXB(XG), RXB(MS), REN(XG), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_256 >= 2 */

/**************************   helper macros (AVX1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0xFF    /*  all satisfy the condition */

#define movsn_rr(RD, XS) /* not portable, do not use outside */             \
        VEX(RXB(RD), RXB(XS),     0x0, 1, 0, 1) EMITB(0x50)                 \
        MRM(REG(RD), MOD(XS), REG(XS))

#define CHECK_MASK(lb, mask, XS) /* destroys Reax */                        \
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
    ADR VEX(0,       RXB(MS),     0x0, 0, 0, 1) EMITB(0xAE)                 \
        MRM(0x02,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mxcsr_st(MD, DD) /* not portable, do not use outside */             \
    ADR VEX(0,       RXB(MD),     0x0, 0, 0, 1) EMITB(0xAE)                 \
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

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMITB(0x04))

#define cvtos_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtos_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvton_ld(XD, MS, DS)                                                \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        VEX(RXB(XD), RXB(XS),     0x0, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XD, XS, mode)                                              \
        rnros_rr(W(XD), W(XS), mode)                                        \
        cvzos_rr(W(XD), W(XD))

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvox_ld(XD, MS, DS) /* not portable, use conditionally */          \
    ADR VEX(RXB(XD), RXB(MS),     0x0, 1, 1, 2) EMITB(0x2C)                 \
        MRM(REG(XD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define mmvox_st(XS, MD, DD) /* not portable, use conditionally */          \
    ADR VEX(RXB(XS), RXB(MD),     0x0, 1, 1, 2) EMITB(0x2E)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

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
        movox_st(XmmF, Oeax, PLAIN)

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
        movox_ld(XmmF, Oeax, PLAIN)

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

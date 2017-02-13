/******************************************************************************/
/* Copyright (c) 2013-2017 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_512V8_H
#define RT_RTARCH_X32_512V8_H

#include "rtarch_x64.h"

#define RT_SIMD_REGS        8
#define RT_SIMD_ALIGN       64
#define RT_SIMD_WIDTH64     8
#define RT_SIMD_SET64(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=v
#define RT_SIMD_WIDTH32     16
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=s[4]=s[5]=s[6]=s[7]=\
                            s[8]=s[9]=s[10]=s[11]=s[12]=s[13]=s[14]=s[15]=v

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_512v8.h: Implementation of x86_64 fp32 AVX1/2 instruction pairs.
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

#if defined (RT_512) && (RT_512 >= 8)

#ifndef RT_RTARCH_X64_256V2_H
#undef  RT_256
#define RT_256  (RT_SIMD_COMPAT_512)
#include "rtarch_x64_256v2.h"
#endif /* RT_RTARCH_X64_256V2_H */

#undef  sregs_sa
#undef  sregs_la

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov (D = S) */

#define movox_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define movox_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x28)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

#define movox_st(XS, MD, DD)                                                \
    ADR VEX(0,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR VEX(1,       RXB(MD),    0x00, 1, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS),    0x02, REG(MD))                                      \
        AUX(SIB(MD), EMITW(VXL(DD)), EMPTY)

/* mmv (G = G mask-merge S, mask: 0 - keeps G, 1 - picks S with elem-size frag)
 * uses Xmm0 implicitly as a mask register, destroys Xmm0, XS unmasked frags */

#define mmvox_rr(XG, XS)                                                    \
    ADR VEX(0,             0, REG(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
    ADR VEX(1,             1, REH(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x80))

#define mmvox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 1, 3) EMITB(0x4A)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x80))

#define mmvox_st(XS, MG, DG)                                                \
    ADR VEX(0,       RXB(MG),    0x00, 1, 1, 2) EMITB(0x2E)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VAL(DG)), EMPTY)                                 \
    ADR VEX(1,       RXB(MG),    0x08, 1, 1, 2) EMITB(0x2E)                 \
        MRM(REG(XS),    0x02, REG(MG))                                      \
        AUX(SIB(MG), EMITW(VXL(DG)), EMPTY)

/* and (G = G & S) */

#define andox_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0x54)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* ann (G = ~G & S) */

#define annox_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0x55)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* orr (G = G | S) */

#define orrox_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0x56)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* orn (G = ~G | S) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, MS, DS)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(MS), W(DS))

/* xor (G = G ^ S) */

#define xorox_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0x57)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* not (G = ~G) */

#define notox_rx(XG)                                                        \
        annox_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (AVX1)   *************/

/* neg (G = -G) */

#define negos_rx(XG)                                                        \
        xorox_ld(W(XG), Mebp, inf_GPC06_32)

/* add (G = G + S), (D = S + T) if (D != S) */

#define addos_rr(XG, XS)                                                    \
        addos3rr(W(XG), W(XG), W(XS))

#define addos_ld(XG, MS, DS)                                                \
        addos3ld(W(XG), W(XG), W(MS), W(DS))

#define addos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define addos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x58)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sub (G = G - S), (D = S - T) if (D != S) */

#define subos_rr(XG, XS)                                                    \
        subos3rr(W(XG), W(XG), W(XS))

#define subos_ld(XG, MS, DS)                                                \
        subos3ld(W(XG), W(XG), W(MS), W(DS))

#define subos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define subos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x5C)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* mul (G = G * S), (D = S * T) if (D != S) */

#define mulos_rr(XG, XS)                                                    \
        mulos3rr(W(XG), W(XG), W(XS))

#define mulos_ld(XG, MS, DS)                                                \
        mulos3ld(W(XG), W(XG), W(MS), W(DS))

#define mulos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define mulos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x59)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* div (G = G / S), (D = S / T) if (D != S) */

#define divos_rr(XG, XS)                                                    \
        divos3rr(W(XG), W(XG), W(XS))

#define divos_ld(XG, MS, DS)                                                \
        divos3ld(W(XG), W(XG), W(MS), W(DS))

#define divos3rr(XD, XS, XT)                                                \
        VEX(0,             0, REG(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))                                      \
        VEX(1,             1, REH(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD), MOD(XT), REG(XT))

#define divos3ld(XD, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 0, 1) EMITB(0x5E)                 \
        MRM(REG(XD),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

/* sqr (D = sqrt S) */

#define sqros_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define sqros_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x51)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cbr (D = cbrt S) */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp (D = 1.0 / S)
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP != 1

#define rceos_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x53)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x53)                 \
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
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x52)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x52)                 \
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

#if (RT_SIMD_COMPAT_512 < 2)

#define addzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x00,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subzm_ri(MG, IS)     /* not portable, do not use outside */         \
        REW(0,       RXB(MG) & (REG(MG) != 4)) EMITB(0x81 | TYP(IS))        \
        MRM(0x05,    0x03,    REG(MG) & (REG(MG) != 4))                     \
        AUX(EMPTY,   EMPTY,   CMD(IS))


#define mivox_st(XS, MD, DD)                                                \
    ADR VEX(0,       RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), EMITW(VAL(DD)), EMPTY)                                 \
    ADR VEX(1,       RXB(MD),    0x00, 0, 0, 1) EMITB(0x29)                 \
        MRM(REG(XS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), EMITW(VXL(DD)), EMPTY)

#define cvyos_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvyos_ld(XD, MS, DS) /* not portable, do not use outside */         \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5A)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

#define cvxqs_rr(XD, XS)     /* not portable, do not use outside */         \
        VEX(0,             0,    0x00, 1, 1, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 1, 1) EMITB(0x5A)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

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

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#define fmaos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvyos_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvyos_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define fmaos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvyos_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvyos_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x10))                              \
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

#elif RT_SIMD_COMPAT_FMS == 1

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#define fmsos_rr(XG, XS, XT)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvyos_rr(W(XG), W(XT))                     /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 1st-pass <- */        \
        cvyos_rr(W(XG), W(XT))                     /* 2nd-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        prmox_rr(W(XT), W(XT), IB(1))              /* 2nd-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define fmsos_ld(XG, XS, MT, DT)                                            \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        cvyos_ld(W(XG), W(MT), W(DT))              /* 1st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addzm_ri(W(MT), IC(0x10))                  /* 1st-pass <- */        \
        cvyos_ld(W(XG), W(MT), W(DT))              /* 2st-pass -> */        \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_rr(W(XG), W(XS))                                              \
        mulqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        cvyos_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subqs_ld(W(XG), Mebp, inf_SCR02(0))                                 \
        cvxqs_rr(W(XG), W(XG))                                              \
        mivox_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        subzm_ri(W(MT), IC(0x10))                  /* 2st-pass <- */        \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#endif /* RT_SIMD_COMPAT_FMS */

#else /* RT_SIMD_COMPAT_512 >= 2 */ /* FMA comes with AVX2 */

/* fma (G = G + S * T)
 * NOTE: x87 fpu-fallbacks for fma/fms use round-to-nearest mode by default,
 * enable RT_SIMD_COMPAT_FMR for current SIMD rounding mode to be honoured */

#if RT_SIMD_COMPAT_FMA <= 1

#define fmaos_rr(XG, XS, XT)                                                \
    ADR VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmaos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0xB8)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMA */

/* fms (G = G - S * T)
 * NOTE: due to final negation being outside of rounding on all Power systems
 * only symmetric rounding modes (RN, RZ) are compatible across all targets */

#if RT_SIMD_COMPAT_FMS <= 1

#define fmsos_rr(XG, XS, XT)                                                \
    ADR VEX(0,             0, REG(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))                                      \
    ADR VEX(1,             1, REH(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG), MOD(XT), REG(XT))

#define fmsos_ld(XG, XS, MT, DT)                                            \
    ADR VEX(0,       RXB(MT), REG(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VAL(DT)), EMPTY)                                 \
    ADR VEX(1,       RXB(MT), REH(XS), 1, 1, 2) EMITB(0xBC)                 \
        MRM(REG(XG),    0x02, REG(MT))                                      \
        AUX(SIB(MT), EMITW(VXL(DT)), EMPTY)

#endif /* RT_SIMD_COMPAT_FMS */

#endif /* RT_SIMD_COMPAT_512 >= 2 */

/* min (G = G < S ? G : S) */

#define minos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0x5D)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* max (G = G > S ? G : S) */

#define maxos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0x5F)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cmp (G = G ? S) */

#define ceqos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x00))

#define cneos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x04))

#define cltos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x01))

#define cleos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x02))

#define cgtos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))                                  \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x06))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x06))

#define cgeos_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))                                  \
        VEX(1,             1, REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeos_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x05))                           \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 0, 1) EMITB(0xC2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x05))

/* cvz (D = fp-to-signed-int S)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x03))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x03))

#define cvzos_rr(XD, XS)     /* round towards zero */                       \
        VEX(0,             0,    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvzos_ld(XD, MS, DS) /* round towards zero */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 2, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cvp (D = fp-to-signed-int S)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XD, XS)     /* round towards +inf */                       \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XD, MS, DS) /* round towards +inf */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x02))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x02))

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
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XD, MS, DS) /* round towards -inf */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x01))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x01))

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
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XD, MS, DS) /* round towards near */                       \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x00))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x00))

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

/**************************   packed integer (AVX1)   *************************/

#if (RT_SIMD_COMPAT_512 < 2)

#define prmox_rr(XD, XS, IT) /* not portable, do not use outside */         \
        VEX(0,             0, REG(XD), 1, 1, 3) EMITB(0x06)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))                               \
        VEX(1,             1, REH(XD), 1, 1, 3) EMITB(0x06)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IT)))

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

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
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        addix_ld(W(XG), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        movix_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

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
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x20))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        subix_ld(W(XG), Mebp, inf_SCR02(0x30))                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        shlix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        shlix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
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
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
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
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shlwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        shrix_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        shrix_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
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
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
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
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwx_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))


#define shron_ri(XG, IS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        shrin_ri(W(XG), W(IS))                                              \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x20))                              \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x20))                              \
        movix_ld(W(XG), Mebp, inf_SCR01(0x30))                              \
        shrin_ld(W(XG), W(MS), W(DS))                                       \
        movix_st(W(XG), Mebp, inf_SCR01(0x30))                              \
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
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x3C))                                    \
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
        movwx_ld(Recx,  Mebp, inf_SCR02(0x20))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x20))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x24))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x24))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x28))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x28))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x2C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x2C))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x30))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x30))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x34))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x34))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x38))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x38))                                    \
        movwx_ld(Recx,  Mebp, inf_SCR02(0x3C))                              \
        shrwn_mx(Mebp,  inf_SCR01(0x3C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_SIMD_COMPAT_512 >= 2 */

/* add (G = G + S) */

#define addox_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 1, 1) EMITB(0xFE)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* sub (G = G - S) */

#define subox_rr(XG, XS)                                                    \
        VEX(0,             0, REG(XG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_ld(XG, MS, DS)                                                \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 1, 1) EMITB(0xFA)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* shl (G = G << S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shlox_ri(XG, IS)                                                    \
        VEX(0,             0, REG(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
        VEX(0,             1, REH(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shlox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 1, 1) EMITB(0xF2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svlox_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(0,             0, REG(XG), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svlox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(0,       RXB(MS), REG(XG), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        VEX(1,       RXB(MS), REH(XG), 1, 1, 2) EMITB(0x47)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* shr (G = G >> S)
 * for maximum compatibility, shift count mustn't exceed elem-size */

#define shrox_ri(XG, IS)                                                    \
        VEX(0,             0, REG(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
        VEX(0,             1, REH(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shrox_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 1, 1) EMITB(0xD2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svrox_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(0,             0, REG(XG), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svrox_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(0,       RXB(MS), REG(XG), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        VEX(1,       RXB(MS), REH(XG), 1, 1, 2) EMITB(0x45)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)


#define shron_ri(XG, IS)                                                    \
        VEX(0,             0, REG(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))                        \
        VEX(0,             1, REH(XG), 1, 1, 1) EMITB(0x72)                 \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x1F))

#define shron_ld(XG, MS, DS) /* loads SIMD, uses 64-bit at given address */ \
    ADR VEX(0,       RXB(MS), REG(XG), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS), REH(XG), 1, 1, 1) EMITB(0xE2)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)

#define svron_rr(XG, XS)     /* variable shift with per-elem count */       \
        VEX(0,             0, REG(XG), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        VEX(1,             1, REH(XG), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG), MOD(XS), REG(XS))

#define svron_ld(XG, MS, DS) /* variable shift with per-elem count */       \
        VEX(0,       RXB(MS), REG(XG), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
        VEX(1,       RXB(MS), REH(XG), 1, 1, 2) EMITB(0x46)                 \
        MRM(REG(XG),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

#endif /* RT_SIMD_COMPAT_512 >= 2 */

/**************************   helper macros (AVX1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in mkj**_** to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE32_512    0x00     /* none satisfy the condition */
#define RT_SIMD_MASK_FULL32_512    0xFF     /*  all satisfy the condition */

#define mkjox_rx(XS, mask, lb)   /* destroys Reax, if S == mask jump lb */  \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(1,             0) EMITB(0x8B)                                   \
        MRM(0x07,       0x03, 0x00)                                         \
        VEX(0,             1,    0x00, 1, 0, 1) EMITB(0x50)                 \
        MRM(0x00,    MOD(XS), REG(XS))                                      \
        REX(0,             1)                                               \
        EMITB(0x03 | (0x08 << ((RT_SIMD_MASK_##mask##32_512 >> 7) << 1)))   \
        MRM(0x00,       0x03, 0x07)                                         \
        cmpwx_ri(Reax, IH(RT_SIMD_MASK_##mask##32_512))                     \
        jeqxx_lb(lb)

/* cvt (D = fp-to-signed-int S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))                                  \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMITB(0x04))                           \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMITB(0x04))

#define cvtos_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvtos_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 1, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cvt (D = signed-int-to-fp S)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XD, XS)                                                    \
        VEX(0,             0,    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        VEX(1,             1,    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD), MOD(XS), REG(XS))

#define cvton_ld(XD, MS, DS)                                                \
    ADR VEX(0,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VAL(DS)), EMPTY)                                 \
    ADR VEX(1,       RXB(MS),    0x00, 1, 0, 1) EMITB(0x5B)                 \
        MRM(REG(XD),    0x02, REG(MS))                                      \
        AUX(SIB(MS), EMITW(VXL(DS)), EMPTY)

/* cvr (D = fp-to-signed-int S)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XD, XS, mode)                                              \
        VEX(0,             0,    0x00, 1, 1, 3) EMITB(0x08)                 \
        MRM(REG(XD), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))                 \
        VEX(1,             1,    0x00, 1, 1, 3) EMITB(0x08)                 \
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
        movox_st(Xmm7, Oeax, PLAIN)

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
        movox_ld(Xmm7, Oeax, PLAIN)

#endif /* RT_512 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_512V8_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

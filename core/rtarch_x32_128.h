/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X32_128_H
#define RT_RTARCH_X32_128_H

#if   RT_ADDRESS == 32

#include "rtarch_x32.h"

#elif RT_ADDRESS == 64

#include "rtarch_x64.h"

#else  /* RT_ADDRESS */

#error "unsupported address size, check RT_ADDRESS in makefiles"

#endif /* RT_ADDRESS */

#define RT_SIMD_REGS        16
#define RT_SIMD_ALIGN       16
#define RT_SIMD_WIDTH32     4
#define RT_SIMD_SET32(s, v) s[0]=s[1]=s[2]=s[3]=v
#define RT_SIMD_WIDTH64     2
#define RT_SIMD_SET64(s, v) s[0]=s[1]=v

#if defined (RT_SIMD_CODE)

#undef  sregs_sa
#undef  sregs_la
#undef  mxcsr_ld

#if defined (RT_128) && (RT_128 != 0)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x32_128.h: Implementation of x86_64 fp32 SSE(1,2,4) instructions.
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
 * XG - SIMD register serving as target and fisrt source
 * XS - SIMD register serving as second source
 * IM - immediate value (smallest size IC is used for shifts)
 *
 * RG - BASE register serving as target and first source
 * RM - BASE register addressing mode (Oeax, M***, I***)
 * DP - displacement value (of given size DP, DF, DG, DH, DV)
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* mandatory escape prefix for some opcodes (must preceed rex) */
#define ESC                                                                 \
        EMITB(0x66)

/* mandatory escape prefix for some opcodes (must preceed rex) */
#define xF3                                                                 \
        EMITB(0xF3)

/* fwait instruction for legacy processors (fix for fstcw) */
#define FWT                                                                 \
        EMITB(0x9B)

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
/**********************************   SSE   ***********************************/
/******************************************************************************/

/**************************   packed generic (SSE1)   *************************/

/* mov */

#define movox_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define movox_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x28)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movox_st(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x29)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* RG is a BASE reg, DP is SIMD-aligned */     \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x8D)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andox_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andox_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x54)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann (~XG & XS) */

#define annox_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annox_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x55)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrox_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrox_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x56)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orn (~XG | XS) */

#define ornox_rr(XG, XS)                                                    \
        notox_rx(W(XG))                                                     \
        orrox_rr(W(XG), W(XS))

#define ornox_ld(XG, RM, DP)                                                \
        notox_rx(W(XG))                                                     \
        orrox_ld(W(XG), W(RM), W(DP))

/* xor */

#define xorox_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorox_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x57)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not */

#define notox_rx(XG)                                                        \
        annox_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (SSE1)   *************/

/* neg */

#define negos_rx(XG)                                                        \
        xorox_ld(W(XG), Mebp, inf_GPC06_32)

/* add */

#define addos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x58)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x5C)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x59)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x5E)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqros_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sqros_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x51)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x53)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define rcsos_rr(XG, XS) /* destroys RM */                                  \
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

#define rseos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x52)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define rssos_rr(XG, XS) /* destroys RM */                                  \
        mulos_rr(W(XS), W(XG))                                              \
        mulos_rr(W(XS), W(XG))                                              \
        subos_ld(W(XS), Mebp, inf_GPC03_32)                                 \
        mulos_ld(W(XS), Mebp, inf_GPC02_32)                                 \
        mulos_rr(W(XG), W(XS))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x5D)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x5F)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeos_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeos_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xC2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/**************************   packed integer (SSE1)   *************************/

#if (RT_128 < 2)

#define fpuws_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuws_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuwn_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDB)                                   \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuwn_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xDB)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_st(RM, DP) /* not portable, do not use outside */             \
FWT ADR REX(0,       RXB(RM)) EMITB(0xD9)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurz_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnzos_rr(XG, XS)     /* round towards zero */                       \
        cvzos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnzos_ld(XG, RM, DP) /* round towards zero */                       \
        cvzos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#define cvzos_rr(XG, XS)     /* round towards zero */                       \
        fpurz_xx()                                                          \
        cvnos_rr(W(XG), W(XS))                                              \
        fpurn_xx()

#define cvzos_ld(XG, RM, DP) /* round towards zero */                       \
        fpurz_xx()                                                          \
        cvnos_ld(W(XG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnpos_rr(XG, XS)     /* round towards +inf */                       \
        cvpos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnpos_ld(XG, RM, DP) /* round towards +inf */                       \
        cvpos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#define cvpos_rr(XG, XS)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnos_rr(W(XG), W(XS))                                              \
        fpurn_xx()

#define cvpos_ld(XG, RM, DP) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnos_ld(W(XG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnmos_rr(XG, XS)     /* round towards -inf */                       \
        cvmos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnmos_ld(XG, RM, DP) /* round towards -inf */                       \
        cvmos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#define cvmos_rr(XG, XS)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnos_rr(W(XG), W(XS))                                              \
        fpurn_xx()

#define cvmos_ld(XG, RM, DP) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnos_ld(W(XG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define rnnos_rr(XG, XS)     /* round towards near */                       \
        cvnos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnnos_ld(XG, RM, DP) /* round towards near */                       \
        cvnos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#define cvnos_rr(XG, XS)     /* round towards near */                       \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x0C))                                    \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define cvnos_ld(XG, RM, DP) /* round towards near */                       \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        cvnos_rr(W(XG), W(XG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XG, XS)     /* round towards near */                       \
        movox_st(W(XS), Mebp, inf_SCR01(0))                                 \
        fpuwn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x0C))                                    \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define cvnon_ld(XG, RM, DP) /* round towards near */                       \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

/* add */

#define addox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define addox_ld(XG, RM, DP)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        addwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* sub */

#define subox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(XS), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define subox_ld(XG, RM, DP)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        stack_st(Reax)                                                      \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x00))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x00))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x04))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x04))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x08))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x08))                              \
        movwx_ld(Reax,  Mebp, inf_SCR02(0x0C))                              \
        subwx_st(Reax,  Mebp, inf_SCR01(0x0C))                              \
        stack_ld(Reax)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shl */

#define shlox_ri(XG, IM)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shlwx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shlox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(RM), W(DP))                                       \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shr */

#define shrox_ri(XG, IM)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrwx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shrox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(RM), W(DP))                                       \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shron_ri(XG, IM)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrwn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shron_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(RM), W(DP))                                       \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   packed integer (SSE2)   *************************/

#else /* RT_128 >= 2 */

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0x0F) EMITB(0xAE)                       \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnzos_rr(XG, XS)     /* round towards zero */                       \
        cvzos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnzos_ld(XG, RM, DP) /* round towards zero */                       \
        cvzos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#else /* RT_128 >= 4 */

#define rnzos_rr(XG, XS)     /* round towards zero */                       \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XG, RM, DP) /* round towards zero */                       \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x03))

#endif /* RT_128 >= 4 */

#define cvzos_rr(XG, XS)     /* round towards zero */                       \
    xF3 REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cvzos_ld(XG, RM, DP) /* round towards zero */                       \
ADR xF3 REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnpos_rr(XG, XS)     /* round towards +inf */                       \
        cvpos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnpos_ld(XG, RM, DP) /* round towards +inf */                       \
        cvpos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#define cvpos_rr(XG, XS)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_rr(W(XG), W(XS))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpos_ld(XG, RM, DP) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_ld(W(XG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDP)

#else /* RT_128 >= 4 */

#define rnpos_rr(XG, XS)     /* round towards +inf */                       \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XG, RM, DP) /* round towards +inf */                       \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cvpos_rr(XG, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XG), W(XS))                                              \
        cvzos_rr(W(XG), W(XG))

#define cvpos_ld(XG, RM, DP) /* round towards +inf */                       \
        rnpos_ld(W(XG), W(RM), W(DP))                                       \
        cvzos_rr(W(XG), W(XG))

#endif /* RT_128 >= 4 */

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnmos_rr(XG, XS)     /* round towards -inf */                       \
        cvmos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnmos_ld(XG, RM, DP) /* round towards -inf */                       \
        cvmos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#define cvmos_rr(XG, XS)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_rr(W(XG), W(XS))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmos_ld(XG, RM, DP) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_ld(W(XG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDM)

#else /* RT_128 >= 4 */

#define rnmos_rr(XG, XS)     /* round towards -inf */                       \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XG, RM, DP) /* round towards -inf */                       \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cvmos_rr(XG, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XG), W(XS))                                              \
        cvzos_rr(W(XG), W(XG))

#define cvmos_ld(XG, RM, DP) /* round towards -inf */                       \
        rnmos_ld(W(XG), W(RM), W(DP))                                       \
        cvzos_rr(W(XG), W(XG))

#endif /* RT_128 >= 4 */

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnnos_rr(XG, XS)     /* round towards near */                       \
        cvnos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rnnos_ld(XG, RM, DP) /* round towards near */                       \
        cvnos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#else /* RT_128 >= 4 */

#define rnnos_rr(XG, XS)     /* round towards near */                       \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XG, RM, DP) /* round towards near */                       \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#endif /* RT_128 >= 4 */

#define cvnos_rr(XG, XS)     /* round towards near */                       \
        cvtos_rr(W(XG), W(XS))

#define cvnos_ld(XG, RM, DP) /* round towards near */                       \
        cvtos_ld(W(XG), W(RM), W(DP))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(XG, XS)     /* round towards near */                       \
        cvton_rr(W(XG), W(XS))

#define cvnon_ld(XG, RM, DP) /* round towards near */                       \
        cvton_ld(W(XG), W(RM), W(DP))

/* add */

#define addox_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xFE)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_ld(XG, RM, DP)                                                \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xFE)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subox_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0xFA)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_ld(XG, RM, DP)                                                \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xFA)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlox_ri(XG, IM)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xF2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrox_ri(XG, IM)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xD2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shron_ri(XG, IM)                                                    \
    ESC REX(0,       RXB(XG)) EMITB(0x0F) EMITB(0x72)                       \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shron_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0xE2)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 2 */

/**************************   helper macros (SSE1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x0F    /*  all satisfy the condition */

#define movsn_rr(XG, XS) /* not portable, do not use outside */             \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x50)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define CHECK_MASK(lb, mask, XG) /* destroys Reax */                        \
        movsn_rr(Reax, W(XG))                                               \
        cmpwx_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
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

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0x0F) EMITB(0xAE)                       \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(RM)) EMITB(0x0F) EMITB(0xAE)                       \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

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

#if (RT_128 < 2)

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XG, XS)                                                    \
        cvtos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rndos_ld(XG, RM, DP)                                                \
        cvtos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#define cvtos_rr(XG, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnos_rr(W(XG), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvtos_ld(XG, RM, DP)                                                \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        cvtos_rr(W(XG), W(XG))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XG, XS)                                                    \
        fpucw_st(Mebp,  inf_SCR02(4))                                       \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnon_rr(W(XG), W(XS))                                              \
        fpucw_ld(Mebp,  inf_SCR02(4))

#define cvton_ld(XG, RM, DP)                                                \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        cvton_rr(W(XG), W(XG))

#else /* RT_128 >= 2 */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rndos_rr(XG, XS)                                                    \
        cvtos_rr(W(XG), W(XS))                                              \
        cvnon_rr(W(XG), W(XG))

#define rndos_ld(XG, RM, DP)                                                \
        cvtos_ld(W(XG), W(RM), W(DP))                                       \
        cvnon_rr(W(XG), W(XG))

#else /* RT_128 >= 4 */

#define rndos_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XG, RM, DP)                                                \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#endif /* RT_128 >= 4 */

#define cvtos_rr(XG, XS)                                                    \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cvtos_ld(XG, RM, DP)                                                \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XG, XS)                                                    \
        REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cvton_ld(XG, RM, DP)                                                \
    ADR REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x5B)                       \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 2 */

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnros_rr(XG, XS, mode)                                              \
        cvros_rr(W(XG), W(XS), mode)                                        \
        cvnon_rr(W(XG), W(XG))

#define cvros_rr(XG, XS, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtos_rr(W(XG), W(XS))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_128 >= 4 */

#define rnros_rr(XG, XS, mode)                                              \
    ESC REX(RXB(XG), RXB(XS)) EMITB(0x0F) EMITB(0x3A) EMITB(0x08)           \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XG, XS, mode)                                              \
        rnros_rr(W(XG), W(XS), mode)                                        \
        cvzos_rr(W(XG), W(XG))

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvox_ld(XG, RM, DP) /* not portable, use conditionally */          \
ADR ESC REX(RXB(XG), RXB(RM)) EMITB(0x0F) EMITB(0x38) EMITB(0x14)           \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 4 */

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

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

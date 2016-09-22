/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_256_H
#define RT_RTARCH_X86_256_H

#if   RT_ADDRESS == 32

#include "rtarch_x86.h"

#elif RT_ADDRESS == 64

#error "unsupported address size for chosen target architecture"

#else  /* RT_ADDRESS */

#error "unsupported address size, check RT_ADDRESS in makefiles"

#endif /* RT_ADDRESS */

#define RT_SIMD_REGS        8
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
 * rtarch_x86_256.h: Implementation of x86:i386 fp32 AVX(1,2) instructions.
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

/* 2-byte VEX prefix, 128/256-bit mode in len, leading 0x0F is implied */
#define VX2(ren, pfx, len)                                                  \
        EMITB(0xC5)                                                         \
        EMITB(0x80 | (len) << 2 | (0x0F - (ren)) << 3 | (pfx))

/* 3-byte VEX prefix, 256-bit mode is encoded, leading 0x0F(xx) in aux */
#define VX3(ren, pfx, aux)                                                  \
        EMITB(0xC4)                                                         \
        EMITB(0xE0 | (aux))                                                 \
        EMITB(0x04 | (0x0F - (ren)) << 3 | (pfx))

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
/**********************************   AVX   ***********************************/
/******************************************************************************/

/**************************   packed generic (AVX1)   *************************/

/* mov */

#define movox_rr(XG, XS)                                                    \
        VX2(0x0,     0, 1) EMITB(0x28)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define movox_ld(XG, RM, DP)                                                \
        VX2(0x0,     0, 1) EMITB(0x28)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movox_st(XG, RM, DP)                                                \
        VX2(0x0,     0, 1) EMITB(0x29)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* RG is a BASE reg, DP is SIMD-aligned */     \
        EMITB(0x8D)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andox_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x54)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define andox_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x54)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann (~XG & XS) */

#define annox_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x55)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define annox_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x55)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrox_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x56)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define orrox_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x56)                                      \
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
        VX2(REG(XG), 0, 1) EMITB(0x57)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define xorox_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x57)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not */

#define notox_rx(XG)                                                        \
        annox_ld(W(XG), Mebp, inf_GPC07)

/**************   packed single precision floating point (AVX1)   *************/

/* neg */

#define negos_rx(XG)                                                        \
        xorox_ld(W(XG), Mebp, inf_GPC06_32)

/* add */

#define addos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x58)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x58)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x5C)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x5C)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x59)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define mulos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x59)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x5E)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define divos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x5E)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqros_rr(XG, XS)                                                    \
        VX2(0x0,     0, 1) EMITB(0x51)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define sqros_ld(XG, RM, DP)                                                \
        VX2(0x0,     0, 1) EMITB(0x51)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(XG, XS)                                                    \
        VX2(0x0,     0, 1) EMITB(0x53)                                      \
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
        VX2(0x0,     0, 1) EMITB(0x52)                                      \
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
        VX2(REG(XG), 0, 1) EMITB(0x5D)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define minos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x5D)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0x5F)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define maxos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0x5F)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeos_rr(XG, XS)                                                    \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeos_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 0, 1) EMITB(0xC2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnzos_rr(XG, XS)     /* round towards zero */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(XG, RM, DP) /* round towards zero */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x03))

#define cvzos_rr(XG, XS)     /* round towards zero */                       \
        VX2(0x0,     2, 1) EMITB(0x5B)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cvzos_ld(XG, RM, DP) /* round towards zero */                       \
        VX2(0x0,     2, 1) EMITB(0x5B)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnpos_rr(XG, XS)     /* round towards +inf */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(XG, RM, DP) /* round towards +inf */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cvpos_rr(XG, XS)     /* round towards +inf */                       \
        rnpos_rr(W(XG), W(XS))                                              \
        cvzos_rr(W(XG), W(XG))

#define cvpos_ld(XG, RM, DP) /* round towards +inf */                       \
        rnpos_ld(W(XG), W(RM), W(DP))                                       \
        cvzos_rr(W(XG), W(XG))

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnmos_rr(XG, XS)     /* round towards -inf */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(XG, RM, DP) /* round towards -inf */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cvmos_rr(XG, XS)     /* round towards -inf */                       \
        rnmos_rr(W(XG), W(XS))                                              \
        cvzos_rr(W(XG), W(XG))

#define cvmos_ld(XG, RM, DP) /* round towards -inf */                       \
        rnmos_ld(W(XG), W(RM), W(DP))                                       \
        cvzos_rr(W(XG), W(XG))

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnnos_rr(XG, XS)     /* round towards near */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(XG, RM, DP) /* round towards near */                       \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

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

/**************************   packed integer (AVX1)   *************************/

#if (RT_256 < 2)

#define prmox_rr(XG, XS, IM) /* not portable, do not use outside */         \
        VX3(REG(XG), 1, 3) EMITB(0x46)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM)))

#define movlx_ld(XG, RM, DP) /* not portable, do not use outside */         \
        VX2(0x0,     0, 0) EMITB(0x28)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movlx_st(XG, RM, DP) /* not portable, do not use outside */         \
        VX2(0x0,     0, 0) EMITB(0x29)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add */

#define addlx_rr(XG, XS)     /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0xFE)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        addlx_rr(W(XG), W(XS))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        addlx_rr(W(XG), W(XS))                                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define addlx_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 1, 0) EMITB(0xFE)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addox_ld(XG, RM, DP)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        addlx_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        addlx_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* sub */

#define sublx_rr(XG, XS)     /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0xFA)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_rr(XG, XS)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        sublx_rr(W(XG), W(XS))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        sublx_rr(W(XG), W(XS))                                              \
        prmox_rr(W(XS), W(XS), IB(1))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define sublx_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 1, 0) EMITB(0xFA)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subox_ld(XG, RM, DP)                                                \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(XG), W(RM), W(DP))                                       \
        movox_st(W(XG), Mebp, inf_SCR02(0))                                 \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x00))                              \
        sublx_ld(W(XG), Mebp, inf_SCR02(0x00))                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        sublx_ld(W(XG), Mebp, inf_SCR02(0x10))                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shl */

#define shllx_ri(XG, IM)     /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0x72)                                      \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlox_ri(XG, IM)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shllx_ri(W(XG), W(IM))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shllx_ri(W(XG), W(IM))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shllx_ld(XG, RM, DP) /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0xF2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shlox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shllx_ld(W(XG), W(RM), W(DP))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shllx_ld(W(XG), W(RM), W(DP))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/* shr */

#define shrlx_ri(XG, IM)     /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0x72)                                      \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrox_ri(XG, IM)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrlx_ri(W(XG), W(IM))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrlx_ri(W(XG), W(IM))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shrlx_ld(XG, RM, DP) /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0xD2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrlx_ld(W(XG), W(RM), W(DP))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrlx_ld(W(XG), W(RM), W(DP))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shrln_ri(XG, IM)     /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0x72)                                      \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shron_ri(XG, IM)                                                    \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrln_ri(W(XG), W(IM))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrln_ri(W(XG), W(IM))                                              \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

#define shrln_ld(XG, RM, DP) /* not portable, do not use outside */         \
        VX2(REG(XG), 1, 0) EMITB(0xE2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shron_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(XG), Mebp, inf_SCR01(0))                                 \
        shrln_ld(W(XG), W(RM), W(DP))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x00))                              \
        movlx_ld(W(XG), Mebp, inf_SCR01(0x10))                              \
        shrln_ld(W(XG), W(RM), W(DP))                                       \
        movlx_st(W(XG), Mebp, inf_SCR01(0x10))                              \
        movox_ld(W(XG), Mebp, inf_SCR01(0))

/**************************   packed integer (AVX2)   *************************/

#else /* RT_256 >= 2 */

/* add */

#define addox_rr(XG, XS)                                                    \
        VX2(REG(XG), 1, 1) EMITB(0xFE)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define addox_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 1, 1) EMITB(0xFE)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subox_rr(XG, XS)                                                    \
        VX2(REG(XG), 1, 1) EMITB(0xFA)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define subox_ld(XG, RM, DP)                                                \
        VX2(REG(XG), 1, 1) EMITB(0xFA)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlox_ri(XG, IM)                                                    \
        VX2(REG(XG), 1, 1) EMITB(0x72)                                      \
        MRM(0x06,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        VX2(REG(XG), 1, 1) EMITB(0xF2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrox_ri(XG, IM)                                                    \
        VX2(REG(XG), 1, 1) EMITB(0x72)                                      \
        MRM(0x02,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrox_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        VX2(REG(XG), 1, 1) EMITB(0xD2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shron_ri(XG, IM)                                                    \
        VX2(REG(XG), 1, 1) EMITB(0x72)                                      \
        MRM(0x04,    MOD(XG), REG(XG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shron_ld(XG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        VX2(REG(XG), 1, 1) EMITB(0xE2)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_256 >= 2 */

/**************************   helper macros (AVX1)   **************************/

/* simd mask
 * compatibility with AVX-512 and ARM-SVE can be achieved by always keeping
 * one hidden SIMD register holding all 1s and using one hidden mask register
 * first in cmp (c**ps) to produce compatible result in target SIMD register
 * then in CHECK_MASK to facilitate branching on a given condition value */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0xFF    /*  all satisfy the condition */

#define movsn_rr(XG, XS) /* not portable, do not use outside */             \
        VX2(0x0,     0, 1) EMITB(0x50)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define CHECK_MASK(lb, mask, XG) /* destroys Reax */                        \
        movsn_rr(Reax, W(XG))                                               \
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

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
        VX2(0x0,     0, 0) EMITB(0xAE)                                      \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
        VX2(0x0,     0, 0) EMITB(0xAE)                                      \
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

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rndos_rr(XG, XS)                                                    \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(XG, RM, DP)                                                \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cvtos_rr(XG, XS)                                                    \
        VX2(0x0,     1, 1) EMITB(0x5B)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cvtos_ld(XG, RM, DP)                                                \
        VX2(0x0,     1, 1) EMITB(0x5B)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(XG, XS)                                                    \
        VX2(0x0,     0, 1) EMITB(0x5B)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))

#define cvton_ld(XG, RM, DP)                                                \
        VX2(0x0,     0, 1) EMITB(0x5B)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define rnros_rr(XG, XS, mode)                                              \
        VX3(0x0,     1, 3) EMITB(0x08)                                      \
        MRM(REG(XG), MOD(XS), REG(XS))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(XG, XS, mode)                                              \
        rnros_rr(W(XG), W(XS), mode)                                        \
        cvzos_rr(W(XG), W(XG))

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvox_ld(XG, RM, DP) /* not portable, use conditionally */          \
        VX3(0x0,     1, 2) EMITB(0x2C)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mmvox_st(XG, RM, DP) /* not portable, use conditionally */          \
        VX3(0x0,     1, 2) EMITB(0x2E)                                      \
        MRM(REG(XG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

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

#endif /* RT_256 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_256_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

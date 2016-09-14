/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X86_128_H
#define RT_RTARCH_X86_128_H

#if   RT_ADDRESS == 32

#include "rtarch_x86.h"

#elif RT_ADDRESS == 64

#error "unsupported address size for chosen target architecture"

#else  /* RT_ADDRESS */

#error "unsupported address size, check RT_ADDRESS in makefiles"

#endif /* RT_ADDRESS */

#define RT_SIMD_REGS        8
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
 * rtarch_x86_128.h: Implementation of x86:i386 fp32 SSE(1,2,4) instructions.
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
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* mandatory escape prefix for some opcodes */
#define ESC                                                                 \
        EMITB(0x66)

/* mandatory escape prefix for some opcodes */
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

/******************************************************************************/
/**********************************   SSE   ***********************************/
/******************************************************************************/

/**************************   packed generic (SSE1)   *************************/

/* mov */

#define movox_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movox_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x28)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movox_st(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x29)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define adrpx_ld(RG, RM, DP) /* RG is a BASE reg, DP is SIMD-aligned */     \
        EMITB(0x8D)                                                         \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and */

#define andox_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andox_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x54)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann */

#define annox_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define annox_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x55)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr */

#define orrox_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrox_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x56)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor */

#define xorox_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorox_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x57)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/**************   packed single precision floating point (SSE1)   *************/

/* add */

#define addos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x58)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5C)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* mul */

#define mulos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x59)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* div */

#define divos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define divos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5E)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sqr */

#define sqros_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define sqros_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x51)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RCP == 0

#define rceos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x53)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define rcsos_rr(RG, RM) /* destroys RM */                                  \
        mulos_rr(W(RM), W(RG))                                              \
        mulos_rr(W(RM), W(RG))                                              \
        addos_rr(W(RG), W(RG))                                              \
        subos_rr(W(RG), W(RM))

#endif /* RT_SIMD_COMPAT_RCP */

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq
 * accuracy/behavior may vary across supported targets, use accordingly */

#if RT_SIMD_COMPAT_RSQ == 0

#define rseos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x52)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define rssos_rr(RG, RM) /* destroys RM */                                  \
        mulos_rr(W(RM), W(RG))                                              \
        mulos_rr(W(RM), W(RG))                                              \
        subos_ld(W(RM), Mebp, inf_GPC03_32)                                 \
        mulos_ld(W(RM), Mebp, inf_GPC02_32)                                 \
        mulos_rr(W(RG), W(RM))

#endif /* RT_SIMD_COMPAT_RSQ */

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define minos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5D)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* max */

#define maxos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define maxos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5F)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cmp */

#define ceqos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define ceqos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#define cneos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define cneos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#define cltos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define cltos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cleos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define cleos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cgtos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x06))

#define cgtos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x06))

#define cgeos_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x05))

#define cgeos_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0xC2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x05))

/**************************   packed integer (SSE1)   *************************/

#if (RT_128 < 2)

#define fpuws_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xD9)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuws_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xD9)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuwn_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xDB)                                                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpuwn_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xDB)                                                         \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0xD9)                                                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define fpucw_st(RM, DP) /* not portable, do not use outside */             \
    FWT EMITB(0xD9)                                                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurz_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x0C7F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnzos_rr(RG, RM)     /* round towards zero */                       \
        cvzos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnzos_ld(RG, RM, DP) /* round towards zero */                       \
        cvzos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#define cvzos_rr(RG, RM)     /* round towards zero */                       \
        fpurz_xx()                                                          \
        cvnos_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvzos_ld(RG, RM, DP) /* round towards zero */                       \
        fpurz_xx()                                                          \
        cvnos_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurp_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x087F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnpos_rr(RG, RM)     /* round towards +inf */                       \
        cvpos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnpos_ld(RG, RM, DP) /* round towards +inf */                       \
        cvpos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#define cvpos_rr(RG, RM)     /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnos_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvpos_ld(RG, RM, DP) /* round towards +inf */                       \
        fpurp_xx()                                                          \
        cvnos_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurm_xx()       /* not portable, do not use outside */             \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        movwx_mi(Mebp,  inf_SCR02(0), IH(0x047F))                           \
        fpucw_ld(Mebp,  inf_SCR02(0))

#define rnmos_rr(RG, RM)     /* round towards -inf */                       \
        cvmos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnmos_ld(RG, RM, DP) /* round towards -inf */                       \
        cvmos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#define cvmos_rr(RG, RM)     /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnos_rr(W(RG), W(RM))                                              \
        fpurn_xx()

#define cvmos_ld(RG, RM, DP) /* round towards -inf */                       \
        fpurm_xx()                                                          \
        cvnos_ld(W(RG), W(RM), W(DP))                                       \
        fpurn_xx()

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#define fpurn_xx()       /* not portable, do not use outside */             \
        fpucw_ld(Mebp,  inf_SCR00)

#define rnnos_rr(RG, RM)     /* round towards near */                       \
        cvnos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnnos_ld(RG, RM, DP) /* round towards near */                       \
        cvnos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#define cvnos_rr(RG, RM)     /* round towards near */                       \
        movox_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuws_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuwn_st(Mebp,  inf_SCR01(0x0C))                                    \
        movox_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnos_ld(RG, RM, DP) /* round towards near */                       \
        movox_ld(W(RG), W(RM), W(DP))                                       \
        cvnos_rr(W(RG), W(RG))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(RG, RM)     /* round towards near */                       \
        movox_st(W(RM), Mebp, inf_SCR01(0))                                 \
        fpuwn_ld(Mebp,  inf_SCR01(0x00))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x00))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x04))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x04))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x08))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x08))                                    \
        fpuwn_ld(Mebp,  inf_SCR01(0x0C))                                    \
        fpuws_st(Mebp,  inf_SCR01(0x0C))                                    \
        movox_ld(W(RG), Mebp, inf_SCR01(0))

#define cvnon_ld(RG, RM, DP) /* round towards near */                       \
        movox_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

/* add */

#define addox_rr(RG, RM)                                                    \
        movox_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(RM), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(RG), Mebp, inf_SCR01(0))

#define addox_ld(RG, RM, DP)                                                \
        movox_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(RG), W(RM), W(DP))                                       \
        movox_st(W(RG), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(RG), Mebp, inf_SCR01(0))

/* sub */

#define subox_rr(RG, RM)                                                    \
        movox_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movox_st(W(RM), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(RG), Mebp, inf_SCR01(0))

#define subox_ld(RG, RM, DP)                                                \
        movox_st(W(RG), Mebp, inf_SCR01(0))                                 \
        movox_ld(W(RG), W(RM), W(DP))                                       \
        movox_st(W(RG), Mebp, inf_SCR02(0))                                 \
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
        movox_ld(W(RG), Mebp, inf_SCR01(0))

/* shl */

#define shlox_ri(RM, IM)                                                    \
        movox_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shlwx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shlwx_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movox_ld(W(RM), Mebp, inf_SCR01(0))

#define shlox_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(RM), W(DP))                                       \
        shlwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shlwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(RG), Mebp, inf_SCR01(0))

/* shr */

#define shrox_ri(RM, IM)                                                    \
        movox_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrwx_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shrwx_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movox_ld(W(RM), Mebp, inf_SCR01(0))

#define shrox_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(RM), W(DP))                                       \
        shrwx_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwx_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(RG), Mebp, inf_SCR01(0))

#define shron_ri(RM, IM)                                                    \
        movox_st(W(RM), Mebp, inf_SCR01(0))                                 \
        shrwn_mi(Mebp,  inf_SCR01(0x00), W(IM))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x04), W(IM))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x08), W(IM))                             \
        shrwn_mi(Mebp,  inf_SCR01(0x0C), W(IM))                             \
        movox_ld(W(RM), Mebp, inf_SCR01(0))

#define shron_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
        movox_st(W(RG), Mebp, inf_SCR01(0))                                 \
        stack_st(Recx)                                                      \
        movwx_ld(Recx,  W(RM), W(DP))                                       \
        shrwn_mx(Mebp,  inf_SCR01(0x00))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x04))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x08))                                    \
        shrwn_mx(Mebp,  inf_SCR01(0x0C))                                    \
        stack_ld(Recx)                                                      \
        movox_ld(W(RG), Mebp, inf_SCR01(0))

/**************************   packed integer (SSE2)   *************************/

#else /* RT_128 >= 2 */

#define mxcsr_ld(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvz (fp-to-signed-int)
 * rounding mode is encoded directly (can be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnzos_rr(RG, RM)     /* round towards zero */                       \
        cvzos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnzos_ld(RG, RM, DP) /* round towards zero */                       \
        cvzos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#else /* RT_128 >= 4 */

#define rnzos_rr(RG, RM)     /* round towards zero */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x03))

#define rnzos_ld(RG, RM, DP) /* round towards zero */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x03))

#endif /* RT_128 >= 4 */

#define cvzos_rr(RG, RM)     /* round towards zero */                       \
    xF3 EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvzos_ld(RG, RM, DP) /* round towards zero */                       \
    xF3 EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvp (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnpos_rr(RG, RM)     /* round towards +inf */                       \
        cvpos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnpos_ld(RG, RM, DP) /* round towards +inf */                       \
        cvpos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#define cvpos_rr(RG, RM)     /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDP)

#define cvpos_ld(RG, RM, DP) /* round towards +inf */                       \
        FCTRL_ENTER(ROUNDP)                                                 \
        cvtos_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDP)

#else /* RT_128 >= 4 */

#define rnpos_rr(RG, RM)     /* round towards +inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x02))

#define rnpos_ld(RG, RM, DP) /* round towards +inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x02))

#define cvpos_rr(RG, RM)     /* round towards +inf */                       \
        rnpos_rr(W(RG), W(RM))                                              \
        cvzos_rr(W(RG), W(RG))

#define cvpos_ld(RG, RM, DP) /* round towards +inf */                       \
        rnpos_ld(W(RG), W(RM), W(DP))                                       \
        cvzos_rr(W(RG), W(RG))

#endif /* RT_128 >= 4 */

/* cvm (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnmos_rr(RG, RM)     /* round towards -inf */                       \
        cvmos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnmos_ld(RG, RM, DP) /* round towards -inf */                       \
        cvmos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#define cvmos_rr(RG, RM)     /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(ROUNDM)

#define cvmos_ld(RG, RM, DP) /* round towards -inf */                       \
        FCTRL_ENTER(ROUNDM)                                                 \
        cvtos_ld(W(RG), W(RM), W(DP))                                       \
        FCTRL_LEAVE(ROUNDM)

#else /* RT_128 >= 4 */

#define rnmos_rr(RG, RM)     /* round towards -inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x01))

#define rnmos_ld(RG, RM, DP) /* round towards -inf */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x01))

#define cvmos_rr(RG, RM)     /* round towards -inf */                       \
        rnmos_rr(W(RG), W(RM))                                              \
        cvzos_rr(W(RG), W(RG))

#define cvmos_ld(RG, RM, DP) /* round towards -inf */                       \
        rnmos_ld(W(RG), W(RM), W(DP))                                       \
        cvzos_rr(W(RG), W(RG))

#endif /* RT_128 >= 4 */

/* cvn (fp-to-signed-int)
 * rounding mode encoded directly (cannot be used in FCTRL blocks)
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnnos_rr(RG, RM)     /* round towards near */                       \
        cvnos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rnnos_ld(RG, RM, DP) /* round towards near */                       \
        cvnos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#else /* RT_128 >= 4 */

#define rnnos_rr(RG, RM)     /* round towards near */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x00))

#define rnnos_ld(RG, RM, DP) /* round towards near */                       \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x00))

#endif /* RT_128 >= 4 */

#define cvnos_rr(RG, RM)     /* round towards near */                       \
        cvtos_rr(W(RG), W(RM))

#define cvnos_ld(RG, RM, DP) /* round towards near */                       \
        cvtos_ld(W(RG), W(RM), W(DP))

/* cvn (signed-int-to-fp)
 * rounding mode encoded directly (cannot be used in FCTRL blocks) */

#define cvnon_rr(RG, RM)     /* round towards near */                       \
        cvton_rr(W(RG), W(RM))

#define cvnon_ld(RG, RM, DP) /* round towards near */                       \
        cvton_ld(W(RG), W(RM), W(DP))

/* add */

#define addox_rr(RG, RM)                                                    \
    ESC EMITB(0x0F) EMITB(0xFE)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addox_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0xFE)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub */

#define subox_rr(RG, RM)                                                    \
    ESC EMITB(0x0F) EMITB(0xFA)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subox_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0xFA)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shl */

#define shlox_ri(RM, IM)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shlox_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
    ESC EMITB(0x0F) EMITB(0xF2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* shr */

#define shrox_ri(RM, IM)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shrox_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
    ESC EMITB(0x0F) EMITB(0xD2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shron_ri(RM, IM)                                                    \
    ESC EMITB(0x0F) EMITB(0x72)                                             \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x1F))

#define shron_ld(RG, RM, DP) /* loads SIMD, uses 1 elem at given address */ \
    ESC EMITB(0x0F) EMITB(0xE2)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
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

#define movsn_rr(RG, RM) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0x50)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        movsn_rr(Reax, W(RG))                                               \
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
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define mxcsr_st(RM, DP) /* not portable, do not use outside */             \
        EMITB(0x0F) EMITB(0xAE)                                             \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#if RT_SIMD_FAST_FCTRL == 0

#define FCTRL_SET(mode)   /* sets given mode into fp control register */    \
        movwx_mi(Mebp, inf_SCR00, IH(RT_SIMD_MODE_##mode << 13 | 0x1F80))   \
        mxcsr_ld(Mebp, inf_SCR00)                                           \

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

#define rndos_rr(RG, RM)                                                    \
        cvtos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rndos_ld(RG, RM, DP)                                                \
        cvtos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#define cvtos_rr(RG, RM)                                                    \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnos_rr(W(RG), W(RM))                                              \
        fpucw_ld(Mebp,  inf_SCR00)

#define cvtos_ld(RG, RM, DP)                                                \
        movox_ld(W(RG), W(RM), W(DP))                                       \
        cvtos_rr(W(RG), W(RG))

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(RG, RM)                                                    \
        fpucw_st(Mebp,  inf_SCR00)                                          \
        mxcsr_st(Mebp,  inf_SCR02(0))                                       \
        shrwx_mi(Mebp,  inf_SCR02(0), IB(3))                                \
        andwx_mi(Mebp,  inf_SCR02(0), IH(0x0C00))                           \
        orrwx_mi(Mebp,  inf_SCR02(0), IB(0x7F))                             \
        fpucw_ld(Mebp,  inf_SCR02(0))                                       \
        cvnon_rr(W(RG), W(RM))                                              \
        fpucw_ld(Mebp,  inf_SCR00)

#define cvton_ld(RG, RM, DP)                                                \
        movox_ld(W(RG), W(RM), W(DP))                                       \
        cvton_rr(W(RG), W(RG))

#else /* RT_128 >= 2 */

/* cvt (fp-to-signed-int)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: ROUNDZ is not supported on pre-VSX Power systems, use cvz
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rndos_rr(RG, RM)                                                    \
        cvtos_rr(W(RG), W(RM))                                              \
        cvnon_rr(W(RG), W(RG))

#define rndos_ld(RG, RM, DP)                                                \
        cvtos_ld(W(RG), W(RM), W(DP))                                       \
        cvnon_rr(W(RG), W(RG))

#else /* RT_128 >= 4 */

#define rndos_rr(RG, RM)                                                    \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(0x04))

#define rndos_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(0x04))

#endif /* RT_128 >= 4 */

#define cvtos_rr(RG, RM)                                                    \
    ESC EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvtos_ld(RG, RM, DP)                                                \
    ESC EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* cvt (signed-int-to-fp)
 * rounding mode comes from fp control register (set in FCTRL blocks)
 * NOTE: only default ROUNDN is supported on pre-VSX Power systems */

#define cvton_rr(RG, RM)                                                    \
        EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cvton_ld(RG, RM, DP)                                                \
        EMITB(0x0F) EMITB(0x5B)                                             \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 2 */

/* cvr (fp-to-signed-int)
 * rounding mode is encoded directly (cannot be used in FCTRL blocks)
 * NOTE: on targets with full-IEEE SIMD fp-arithmetic the ROUND*_F mode
 * isn't always taken into account when used within full-IEEE ASM block
 * NOTE: due to compatibility with legacy targets, SIMD fp-to-int
 * round instructions are only accurate within 32-bit signed int range */

#if (RT_128 < 4)

#define rnros_rr(RG, RM, mode)                                              \
        cvros_rr(W(RG), W(RM), mode)                                        \
        cvnon_rr(W(RG), W(RG))

#define cvros_rr(RG, RM, mode)                                              \
        FCTRL_ENTER(mode)                                                   \
        cvtos_rr(W(RG), W(RM))                                              \
        FCTRL_LEAVE(mode)

#else /* RT_128 >= 4 */

#define rnros_rr(RG, RM, mode)                                              \
    ESC EMITB(0x0F) EMITB(0x3A) EMITB(0x08)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(RT_SIMD_MODE_##mode&3))

#define cvros_rr(RG, RM, mode)                                              \
        rnros_rr(W(RG), W(RM), mode)                                        \
        cvzos_rr(W(RG), W(RG))

/* mmv
 * uses Xmm0 implicitly as a mask register */

#define mmvox_ld(RG, RM, DP) /* not portable, use conditionally */          \
    ESC EMITB(0x0F) EMITB(0x38) EMITB(0x14)                                 \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_128 >= 4 */

/* sregs */

#define sregs_sa() /* save all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_st(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_st(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_st(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_st(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_st(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_st(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_st(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_st(Xmm7, Oeax, PLAIN)

#define sregs_la() /* load all SIMD regs, destroys Reax */                  \
        movxx_ld(Reax, Mebp, inf_REGS)                                      \
        movox_ld(Xmm0, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_ld(Xmm1, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_ld(Xmm2, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_ld(Xmm3, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_ld(Xmm4, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_ld(Xmm5, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_ld(Xmm6, Oeax, PLAIN)                                         \
        addxx_ri(Reax, IB(RT_SIMD_WIDTH*4))                                 \
        movox_ld(Xmm7, Oeax, PLAIN)

#endif /* RT_128 */

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_X86_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

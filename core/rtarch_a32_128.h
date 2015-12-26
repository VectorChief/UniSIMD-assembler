/******************************************************************************/
/* Copyright (c) 2013-2015 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A32_128_H
#define RT_RTARCH_A32_128_H

#include "rtarch_a32.h"

#define RT_SIMD_WIDTH       4
#define RT_SIMD_ALIGN       16
#define RT_SIMD_SET(s, v)   s[0]=s[1]=s[2]=s[3]=v

#if defined (RT_SIMD_CODE)

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a32_128.h: Implementation of A32 SIMD instructions.
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
 * cmdp*_mr - applies [cmd] to [p]acked: [m]emory   from [r]egister
 * cmdp*_st - applies [cmd] to [p]acked: as above (arg list as cmdp*_ld)
 *
 * cmdpx_** - applies [cmd] to [p]acked unsigned integer args, [x] - default
 * cmdpn_** - applies [cmd] to [p]acked   signed integer args, [n] - negatable
 * cmdps_** - applies [cmd] to [p]acked floating point   args, [s] - scalable
 *
 * The cmdp*_** instructions are intended for SPMD programming model
 * and can potentially be configured per target to work with 32-bit/64-bit
 * data-elements (integers/pointers, floating point).
 * In this model data-paths are fixed-width, core and SIMD data-elements are
 * width-compatible, code-path divergence is handled via CHECK_MASK macro.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* registers    REG */

#define Tmm1    0x1F                    /* v31 */

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
#define XmmE    0x0E, 0x00, EMPTY       /* v14 */
#define XmmF    0x0F, 0x00, EMPTY       /* v15 */

/******************************************************************************/
/**********************************   MPE   ***********************************/
/******************************************************************************/

/**************************   packed generic (NEON)   *************************/

/* mov */

#define movpx_rr(RG, RM)                                                    \
        EMITW(0x4EA01C00 | MRM(REG(RG), REG(RM), REG(RM)))

#define movpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(REG(RG), MOD(RM), 0x00) | TYP(DP))

#define movpx_st(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3D800000 | MRM(REG(RG), MOD(RM), 0x00) | TYP(DP))

#define adrpx_ld(RG, RM, DP) /* RG is a core reg, DP is SIMD-aligned */     \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x52800000 | MRM(REG(RG), 0x00,    0x00) | TYP(DP) >> 1)      \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), MOD(RM))

/* and */

#define andpx_rr(RG, RM)                                                    \
        EMITW(0x4E201C00 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define andpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E201C00 | MRM(REG(RG), REG(RG), Tmm1))

/* ann */

#define annpx_rr(RG, RM)                                                    \
        EMITW(0x4E601C00 | MRM(REG(RG), REG(RM), REG(RG)))                  \

#define annpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E601C00 | MRM(REG(RG), Tmm1,    REG(RG)))

/* orr */

#define orrpx_rr(RG, RM)                                                    \
        EMITW(0x4EA01C00 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define orrpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4EA01C00 | MRM(REG(RG), REG(RG), Tmm1))

/* xor */

#define xorpx_rr(RG, RM)                                                    \
        EMITW(0x6E201C00 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define xorpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6E201C00 | MRM(REG(RG), REG(RG), Tmm1))

/**************   packed single precision floating point (NEON)   *************/

/* add */

#define addps_rr(RG, RM)                                                    \
        EMITW(0x4E20D400 | MRM(REG(RG), REG(RG), REG(RM)))

#define addps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E20D400 | MRM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subps_rr(RG, RM)                                                    \
        EMITW(0x4EA0D400 | MRM(REG(RG), REG(RG), REG(RM)))

#define subps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4EA0D400 | MRM(REG(RG), REG(RG), Tmm1))

/* mul */

#define mulps_rr(RG, RM)                                                    \
        EMITW(0x6E20DC00 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6E20DC00 | MRM(REG(RG), REG(RG), Tmm1))

/* div */

#define divps_rr(RG, RM)                                                    \
        EMITW(0x6E20FC00 | MRM(REG(RG), REG(RG), REG(RM)))

#define divps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6E20FC00 | MRM(REG(RG), REG(RG), Tmm1))

/* sqr */

#define sqrps_rr(RG, RM)                                                    \
        EMITW(0x6EA1F800 | MRM(REG(RG), REG(RM), 0x00))

#define sqrps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6EA1F800 | MRM(REG(RG), Tmm1,    0x00))

/* cbr */

        /* cbe, cbs, cbr defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rcp */

#define rceps_rr(RG, RM)                                                    \
        EMITW(0x4EA1D800 | MRM(REG(RG), REG(RM), 0x00))

#define rcsps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0x4E20FC00 | MRM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0x6E20DC00 | MRM(REG(RG), REG(RG), REG(RM)))

        /* rcp defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* rsq */

#define rseps_rr(RG, RM)                                                    \
        EMITW(0x6EA1D800 | MRM(REG(RG), REG(RM), 0x00))

#define rssps_rr(RG, RM) /* destroys RM */                                  \
        EMITW(0x6E20DC00 | MRM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0x4EA0FC00 | MRM(REG(RM), REG(RM), REG(RG)))                  \
        EMITW(0x6E20DC00 | MRM(REG(RG), REG(RG), REG(RM)))

        /* rsq defined in rtbase.h
         * under "COMMON SIMD INSTRUCTIONS" section */

/* min */

#define minps_rr(RG, RM)                                                    \
        EMITW(0x4EA0F400 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define minps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4EA0F400 | MRM(REG(RG), REG(RG), Tmm1))

/* max */

#define maxps_rr(RG, RM)                                                    \
        EMITW(0x4E20F400 | MRM(REG(RG), REG(RG), REG(RM)))                  \

#define maxps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E20F400 | MRM(REG(RG), REG(RG), Tmm1))

/* cmp */

#define ceqps_rr(RG, RM)                                                    \
        EMITW(0x4E20E400 | MRM(REG(RG), REG(RG), REG(RM)))

#define ceqps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E20E400 | MRM(REG(RG), REG(RG), Tmm1))

#define cneps_rr(RG, RM)                                                    \
        EMITW(0x4E20E400 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0x6E205800 | MRM(REG(RG), REG(RG), 0x00))

#define cneps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E20E400 | MRM(REG(RG), REG(RG), Tmm1))                     \
        EMITW(0x6E205800 | MRM(REG(RG), REG(RG), 0x00))

#define cltps_rr(RG, RM)                                                    \
        EMITW(0x6EA0E400 | MRM(REG(RG), REG(RM), REG(RG)))

#define cltps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6EA0E400 | MRM(REG(RG), Tmm1,    REG(RG)))

#define cleps_rr(RG, RM)                                                    \
        EMITW(0x6E20E400 | MRM(REG(RG), REG(RM), REG(RG)))

#define cleps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6E20E400 | MRM(REG(RG), Tmm1,    REG(RG)))

#define cgtps_rr(RG, RM)                                                    \
        EMITW(0x6EA0E400 | MRM(REG(RG), REG(RG), REG(RM)))

#define cgtps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6EA0E400 | MRM(REG(RG), REG(RG), Tmm1))

#define cgeps_rr(RG, RM)                                                    \
        EMITW(0x6E20E400 | MRM(REG(RG), REG(RG), REG(RM)))

#define cgeps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6E20E400 | MRM(REG(RG), REG(RG), Tmm1))

/**************************   packed integer (NEON)   *************************/

/* cvt
 * rounding mode comes from fp control register (set in FCTRL blocks) */

#define cvtps_rr(RG, RM)                                                    \
        EMITW(0x6EA19800 | MRM(REG(RG), REG(RM), 0x00))                     \
        EMITW(0x4E21A800 | MRM(REG(RG), REG(RG), 0x00))

#define cvtps_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6EA19800 | MRM(REG(RG), Tmm1,    0x00))                     \
        EMITW(0x4E21A800 | MRM(REG(RG), REG(RG), 0x00))

#define cvtpn_rr(RG, RM)                                                    \
        EMITW(0x4E21D800 | MRM(REG(RG), REG(RM), 0x00))

#define cvtpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E21D800 | MRM(REG(RG), Tmm1,    0x00))

/* cvx
 * rounding mode is encoded directly (not to be used in FCTRL blocks) */

#define cvnpn_rr(RG, RM)     /* round to nearest */                         \
        cvtpn_rr(W(RG), W(RM))

#define cvnpn_ld(RG, RM, DP) /* round to nearest */                         \
        cvtpn_ld(W(RG), W(RM), W(DP))

/* add */

#define addpx_rr(RG, RM)                                                    \
        EMITW(0x4EA08400 | MRM(REG(RG), REG(RG), REG(RM)))

#define addpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4EA08400 | MRM(REG(RG), REG(RG), Tmm1))

/* sub */

#define subpx_rr(RG, RM)                                                    \
        EMITW(0x6EA08400 | MRM(REG(RG), REG(RG), REG(RM)))

#define subpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x6EA08400 | MRM(REG(RG), REG(RG), Tmm1))

/* shl */

#define shlpx_ri(RM, IM)                                                    \
        EMITW(0x4F205400 | MRM(REG(RM), REG(RM), 0x00) |                    \
                                                 (0x1F & VAL(IM)) << 16)

#define shlpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E040400 | MRM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE04400 | MRM(REG(RG), REG(RG), Tmm1))

/* shr */

#define shrpx_ri(RM, IM) /* emit shift-left for zero-immediate args */      \
        EMITW(0x4F200400 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (+(VAL(IM) == 0) & 0x00005000) | (+(VAL(IM) != 0) & 0x20000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IM)) << 16)

#define shrpx_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E040400 | MRM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EA0B800 | MRM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EE04400 | MRM(REG(RG), REG(RG), Tmm1))

#define shrpn_ri(RM, IM) /* emit shift-left for zero-immediate args */      \
        EMITW(0x4F200400 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (+(VAL(IM) == 0) & 0x00005000) | (+(VAL(IM) != 0) & 0x00000000) |   \
        /* if true ^ equals to -1 (not 1) */     (0x1F &-VAL(IM)) << 16)

#define shrpn_ld(RG, RM, DP)                                                \
        AUX(SIB(RM), CMD(DP), EMPTY)                                        \
        EMITW(0x3DC00000 | MRM(Tmm1,    MOD(RM), 0x00) | TYP(DP))           \
        EMITW(0x4E040400 | MRM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x6EA0B800 | MRM(Tmm1,    Tmm1,    0x00))                     \
        EMITW(0x4EE04400 | MRM(REG(RG), REG(RG), Tmm1))

/**************************   helper macros (NEON)   **************************/

/* simd mask */

#define RT_SIMD_MASK_NONE       0x00    /* none satisfy the condition */
#define RT_SIMD_MASK_FULL       0x04    /*  all satisfy the condition */

#define movms_rr(RG, RM) /* not portable, do not use outside */             \
        EMITW(0x4EB1B800 | MRM(Tmm1,    REG(RM), 0x00))                     \
        EMITW(0x0E043C00 | MRM(REG(RG), Tmm1,    0x00))

#define CHECK_MASK(lb, mask, RG) /* destroys Reax */                        \
        movms_rr(Reax, W(RG))                                               \
        addxx_ri(Reax, IB(RT_SIMD_MASK_##mask))                             \
        jezxx_lb(lb)

/* simd mode
 * set in FCTRL blocks (cannot be nested) */

#define RT_SIMD_MODE_ROUNDN     0x00    /* round to nearest */
#define RT_SIMD_MODE_ROUNDM     0x02    /* round towards minus infinity */
#define RT_SIMD_MODE_ROUNDP     0x01    /* round towards plus  infinity */
#define RT_SIMD_MODE_ROUNDZ     0x03    /* round towards zero */

#define fpscr_ld(RG) /* not portable, do not use outside */                 \
        EMITW(0xD51B4400 | MRM(REG(RG), 0x00,    0x00))

#define fpscr_st(RG) /* not portable, do not use outside */                 \
        EMITW(0xD53B4400 | MRM(REG(RG), 0x00,    0x00))

#define FCTRL_ENTER(mode) /* destroys Reax */                               \
        fpscr_st(Reax)                                                      \
        movxx_st(Reax, Mebp, inf_FCTRL)                                     \
        orrxx_ri(Reax, IW(RT_SIMD_MODE_##mode << 22))                       \
        fpscr_ld(Reax)

#define FCTRL_LEAVE(mode) /* destroys Reax */                               \
        movxx_ld(Reax, Mebp, inf_FCTRL)                                     \
        fpscr_ld(Reax)

/* cvr
 * rounding mode is encoded directly (not to be used in FCTRL blocks) */

#define cvrps_rr(RG, RM, mode)                                              \
        EMITW(0x4E21A800 | MRM(REG(RG), REG(RM), 0x00) |                    \
        (RT_SIMD_MODE_##mode & 1) << 23 | (RT_SIMD_MODE_##mode & 2) << 11)

#endif /* RT_SIMD_CODE */

#endif /* RT_RTARCH_A32_128_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

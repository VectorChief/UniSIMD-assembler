/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_PHB_H
#define RT_RTARCH_PHB_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_pHB.h: Implementation of POWER half+byte BASE instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdhx_ri - applies [cmd] to [r]egister from [i]mmediate
 * cmdhx_mi - applies [cmd] to [m]emory   from [i]mmediate
 * cmdhx_rz - applies [cmd] to [r]egister from [z]ero-arg
 * cmdhx_mz - applies [cmd] to [m]emory   from [z]ero-arg
 *
 * cmdhx_rm - applies [cmd] to [r]egister from [m]emory
 * cmdhx_ld - applies [cmd] as above
 * cmdhx_mr - applies [cmd] to [m]emory   from [r]egister
 * cmdhx_st - applies [cmd] as above (arg list as cmdhx_ld)
 *
 * cmdhx_rr - applies [cmd] to [r]egister from [r]egister
 * cmdhx_mm - applies [cmd] to [m]emory   from [m]emory
 * cmdhx_rx - applies [cmd] to [r]egister (one-operand cmd)
 * cmdhx_mx - applies [cmd] to [m]emory   (one-operand cmd)
 *
 * cmdhx_rx - applies [cmd] to [r]egister from x-register
 * cmdhx_mx - applies [cmd] to [m]emory   from x-register
 * cmdhx_xr - applies [cmd] to x-register from [r]egister
 * cmdhx_xm - applies [cmd] to x-register from [m]emory
 *
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 *
 * cmd*z_** - applies [cmd] while setting condition flags, [z] - zero flag.
 * Regular cmd*x_**, cmd*n_** instructions may or may not set flags depending
 * on the target architecture, thus no assumptions can be made for jezxx/jnzxx.
 *
 * Interpretation of instruction parameters:
 *
 * upper-case params have triplet structure and require W to pass-forward
 * lower-case params are singular and can be used/passed as such directly
 *
 * RD - BASE register serving as destination only, if present
 * RG - BASE register serving as destination and first source
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

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movhx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movhx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TWxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MD), VAL(DD), B1(DD), OH(DD)))

#define movhx_rr(RD, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RD), REG(RS), REG(RS)))

#define movhx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), PH(DS)))

#define movhn_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), PS(DS)))

#define movhx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RS), MOD(MD), VAL(DD), B1(DD), OH(DD)))

/* and (G = G & S)
 * set-flags: undefined (*x), yes (*z) */

#define andhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        /* if true ^ equals to -1 (not 1) */

#define andhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define andhx_rr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))

#define andhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))

#define andhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define andhx_mr(MG, DG, RS)                                                \
        andhx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define andhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        /* if true ^ equals to -1 (not 1) */

#define andhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define andhz_rr(RG, RS)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RS)))

#define andhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define andhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define andhz_rr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define andhz_mr(MG, DG, RS)                                                \
        andhz_st(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhx_ri(W(RG), W(IS))

#define annhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define annhx_rr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))

#define annhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))

#define annhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define annhx_mr(MG, DG, RS)                                                \
        annhx_st(W(RS), W(MG), W(DG))


#define annhz_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhz_ri(W(RG), W(IS))

#if RT_BASE_COMPAT_ZFL == 0

#define annhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define annhz_rr(RG, RS)                                                    \
        EMITW(0x7C000079 | MSM(REG(RG), REG(RS), REG(RG)))

#define annhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000079 | MSM(REG(RG), TMxx,    REG(RG)))

#define annhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000079 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define annhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define annhz_rr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define annhz_mr(MG, DG, RS)                                                \
        annhz_st(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

#define orrhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define orrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define orrhx_rr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))

#define orrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define orrhx_mr(MG, DG, RS)                                                \
        orrhx_st(W(RS), W(MG), W(DG))


#define orrhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define orrhz_rr(RG, RS)                                                    \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), TMxx))

#define orrhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000379 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define orrhz_rr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define orrhz_mr(MG, DG, RS)                                                \
        orrhz_st(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhx_ri(W(RG), W(IS))

#define ornhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define ornhx_rr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define ornhx_mr(MG, DG, RS)                                                \
        ornhx_st(W(RS), W(MG), W(DG))


#define ornhz_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhz_ri(W(RG), W(IS))

#define ornhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define ornhz_rr(RG, RS)                                                    \
        EMITW(0x7C000339 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000339 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000339 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define ornhz_rr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define ornhz_mr(MG, DG, RS)                                                \
        ornhz_st(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

#define xorhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        /* if true ^ equals to -1 (not 1) */

#define xorhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define xorhx_rr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))

#define xorhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define xorhx_mr(MG, DG, RS)                                                \
        xorhx_st(W(RS), W(MG), W(DG))


#define xorhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define xorhz_rr(RG, RS)                                                    \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), TMxx))

#define xorhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000279 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define xorhz_rr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define xorhz_mr(MG, DG, RS)                                                \
        xorhz_st(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define nothx_rx(RG)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RG), REG(RG), REG(RG)))

#define nothx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TWxx,    TWxx,    TWxx))                     \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define neghx_rx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))

#define neghx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))


#if RT_BASE_COMPAT_ZFL == 0

#define neghz_rx(RG)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RG), 0x00,    REG(RG)))

#define neghz_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define neghz_rx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define neghz_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

#define addhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        /* if true ^ equals to -1 (not 1) */

#define addhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define addhx_rr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addhn_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define addhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))    \
        /* if true ^ equals to -1 (not 1) */

#define addhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define addhz_rr(RG, RS)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define addhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define addhz_rr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define addhz_mr(MG, DG, RS)                                                \
        addhz_st(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

#define subhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define subhx_rr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subhn_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define subhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define subhz_rr(RG, RS)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define subhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define subhz_rr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define subhz_mr(MG, DG, RS)                                                \
        subhz_st(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), Tecx))

#define shlhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhx_ri(RG, IS)                                                    \
        EMITW(0x54000000 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)

#define shlhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x54000000 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), TMxx))

#define shlhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhx_mr(MG, DG, RS)                                                \
        shlhx_st(W(RS), W(MG), W(DG))


#define shlhz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), Tecx))

#define shlhz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhz_ri(RG, IS)                                                    \
        EMITW(0x54000001 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)

#define shlhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x54000001 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlhz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), TMxx))

#define shlhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhz_mr(MG, DG, RS)                                                \
        shlhz_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), Tecx))

#define shrhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhx_ri(RG, IS)                                                    \
        EMITW(0x5400003E | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6)

#define shrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x5400003E | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), TMxx))

#define shrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhx_mr(MG, DG, RS)                                                \
        shrhx_st(W(RS), W(MG), W(DG))


#define shrhz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), Tecx))

#define shrhz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhz_ri(RG, IS)                                                    \
        EMITW(0x5400003F | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6)

#define shrhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x5400003F | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrhz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), TMxx))

#define shrhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhz_mr(MG, DG, RS)                                                \
        shrhz_st(W(RS), W(MG), W(DG))


#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), Tecx))

#define shrhn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PS(DG)))  \
        EMITW(0x7C000630 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhn_ri(RG, IS)                                                    \
        EMITW(0x7C000670 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F))

#define shrhn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PS(DG)))  \
        EMITW(0x7C000670 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F))           \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrhn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), TMxx))

#define shrhn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PS(DG)))  \
        EMITW(0x7C000630 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhn_mr(MG, DG, RS)                                                \
        shrhn_st(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TIxx))

#define mulhx_rr(RG, RS)                                                    \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))

#define mulhn_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000016 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000096 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))


#define mulhp_xr(RS)     /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulhp_xm(MS, DS) /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divhx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), TIxx))

#define divhx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), REG(RS)))

#define divhx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), TMxx))


#define divhn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TIxx))

#define divhn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), REG(RS)))

#define divhn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TMxx))


#define prehx_xx()          /* to be placed immediately prior divhx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prehn_xx()          /* to be placed immediately prior divhn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    REG(RS)))

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    TMxx))


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    REG(RS)))

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    TMxx))


#define divhp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divhn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divhp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divhn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remhx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhx_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remhx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhx_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remhx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remhn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhn_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remhn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhn_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remhn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remhx_xx()          /* to be placed immediately prior divhx_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remhn_xx()          /* to be placed immediately prior divhn_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhn_xr(RS)        /* to be placed immediately after divhn_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remhn_xm(MS, DS)    /* to be placed immediately after divhn_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjhx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, hz_rx)                                               \
        CMJ(cc, lb)

#define arjhx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, hz_mx)                                        \
        CMJ(cc, lb)

#define arjhx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, hz_ri)                                        \
        CMJ(cc, lb)

#define arjhx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, hz_mi)                                 \
        CMJ(cc, lb)

#define arjhx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, hz_rr)                                        \
        CMJ(cc, lb)

#define arjhx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, hz_ld)                                 \
        CMJ(cc, lb)

#define arjhx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, hz_st)                                 \
        CMJ(cc, lb)

#define arjhx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjhx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjhx_rz(RS, cc, lb)                                                \
        cmjhx_ri(W(RS), IC(0), cc, lb)

#define cmjhx_mz(MS, DS, cc, lb)                                            \
        cmjhx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjhn_mz(MS, DS, cc, lb)                                            \
        cmjhn_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjhx_ri(RS, IT, cc, lb)                                            \
        CWI(cc, MOD(RS), REG(RS), W(IT), lb)

#define cmjhx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        CWI(cc, %%r24,   TMxx,    W(IT), lb)

#define cmjhn_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        CWI(cc, %%r24,   TMxx,    W(IT), lb)

#define cmjhx_rr(RS, RT, cc, lb)                                            \
        CWR(cc, MOD(RS), MOD(RT), lb)

#define cmjhx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PH(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjhn_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PS(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjhx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

#define cmjhn_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmphx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))

#define cmphn_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))

#define cmphx_rr(RS, RT)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PH(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphn_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PS(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

#define cmphn_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_PHB_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

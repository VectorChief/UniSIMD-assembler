/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
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
 * cmd**Z** - applies [cmd] while setting condition flags, [Z] - zero flag.
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

/************************* 16-bit subset instructions *************************/

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

#define movhx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RS), MOD(MD), VAL(DD), B1(DD), OH(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))

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

#define andhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))

#define andhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define andhxZrr(RG, RS)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RS)))

#define andhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define andhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define andhxZrr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define andhxZmr(MG, DG, RS)                                                \
        andhxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

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


#define annhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhxZri(W(RG), W(IS))

#if RT_BASE_COMPAT_ZFL == 0

#define annhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define annhxZrr(RG, RS)                                                    \
        EMITW(0x7C000079 | MSM(REG(RG), REG(RS), REG(RG)))

#define annhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000079 | MSM(REG(RG), TMxx,    REG(RG)))

#define annhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000079 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define annhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define annhxZrr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define annhxZmr(MG, DG, RS)                                                \
        annhxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

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


#define orrhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define orrhxZrr(RG, RS)                                                    \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), TMxx))

#define orrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000379 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define orrhxZrr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define orrhxZmr(MG, DG, RS)                                                \
        orrhxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

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


#define ornhxZri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhxZri(W(RG), W(IS))

#define ornhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define ornhxZrr(RG, RS)                                                    \
        EMITW(0x7C000339 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000339 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000339 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define ornhxZrr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define ornhxZmr(MG, DG, RS)                                                \
        ornhxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))

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


#define xorhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define xorhxZrr(RG, RS)                                                    \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), TMxx))

#define xorhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000279 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define xorhxZrr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define xorhxZmr(MG, DG, RS)                                                \
        xorhxZst(W(RS), W(MG), W(DG))

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
 * set-flags: undefined (*_*), yes (*Z*) */

#define neghx_rx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))

#define neghx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))


#if RT_BASE_COMPAT_ZFL == 0

#define neghxZrx(RG)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RG), 0x00,    REG(RG)))

#define neghxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define neghxZrx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define neghxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))

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

#define addhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define addhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))

#define addhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define addhxZrr(RG, RS)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define addhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define addhxZrr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define addhxZmr(MG, DG, RS)                                                \
        addhxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))

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

#define subhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define subhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))

#define subhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define subhxZrr(RG, RS)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define subhxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define subhxZrr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define subhxZmr(MG, DG, RS)                                                \
        subhxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
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


#define shlhxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), Tecx))

#define shlhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhxZri(RG, IS)                                                    \
        EMITW(0x54000001 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)

#define shlhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x54000001 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), TMxx))

#define shlhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shlhxZmr(MG, DG, RS)                                                \
        shlhxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
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


#define shrhxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), Tecx))

#define shrhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhxZri(RG, IS)                                                    \
        EMITW(0x5400003F | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6)

#define shrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x5400003F | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), TMxx))

#define shrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PH(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhxZmr(MG, DG, RS)                                                \
        shrhxZst(W(RS), W(MG), W(DG))


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


#define shrhnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), Tecx))

#define shrhnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PS(DG)))  \
        EMITW(0x7C000631 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhnZri(RG, IS)                                                    \
        EMITW(0x7C000671 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F))

#define shrhnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PS(DG)))  \
        EMITW(0x7C000671 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F))           \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrhnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), TMxx))

#define shrhnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PS(DG)))  \
        EMITW(0x7C000631 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OH(DG)))

#define shrhnZmr(MG, DG, RS)                                                \
        shrhnZst(W(RS), W(MG), W(DG))

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


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x5400003E | MSM(Tedx,    Teax,    16) | 16 << 6)

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x5400003E | MSM(Tedx,    Teax,    16) | 16 << 6)


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x7C000670 | MSM(Tedx,    Teax,    16))

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x7C000670 | MSM(Tedx,    Teax,    16))


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

#if RT_BASE_COMPAT_REM == 0

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

#else /* RT_BASE_COMPAT_REM != 0 */

#define remhx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), TIxx))

#define remhx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), REG(RS)))

#define remhx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), TMxx))


#define remhn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), TIxx))

#define remhn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), REG(RS)))

#define remhn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PS(DS)))  \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), TMxx))


#define remhx_xx()          /* to be placed immediately prior divhx_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
        EMITW(0x7C000216 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
        EMITW(0x7C000216 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remhn_xx()          /* to be placed immediately prior divhn_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhn_xr(RS)        /* to be placed immediately after divhn_xr */   \
        EMITW(0x7C000616 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remhn_xm(MS, DS)    /* to be placed immediately after divhn_xm */   \
        EMITW(0x7C000616 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#endif /* RT_BASE_COMPAT_REM != 0 */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define add_n   AM3
#define sub_n   AM4

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjhx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, h, Zrx)                                              \
        CMJ(cc, lb)

#define arjhx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, h, Zmx)                                       \
        CMJ(cc, lb)

#define arjhx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, h, Zri)                                       \
        CMJ(cc, lb)

#define arjhx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, h, Zmi)                                \
        CMJ(cc, lb)

#define arjhx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, h, Zrr)                                       \
        CMJ(cc, lb)

#define arjhx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, h, Zld)                                \
        CMJ(cc, lb)

#define arjhx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, h, Zst)                                \
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

#define cmjhx_ri(RS, IT, cc, lb)                                            \
        CWI(cc, MOD(RS), REG(RS), W(IT), lb)

#define cmjhx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        CWI(cc, %%r24,   TMxx,    W(IT), lb)

#define cmjhx_rr(RS, RT, cc, lb)                                            \
        CWR(cc, MOD(RS), MOD(RT), lb)

#define cmjhx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PH(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjhx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmphx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))

#define cmphx_rr(RS, RT)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PH(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmphx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PH(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

/************************** 8-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movbx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movbx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TWxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MD), VAL(DD), B1(DD), OB(DD)))

#define movbx_rr(RD, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RD), REG(RS), REG(RS)))

#define movbx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), PB(DS)))

#define movbx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RS), MOD(MD), VAL(DD), B1(DD), OB(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))

#define andbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define andbx_rr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))

#define andbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))

#define andbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define andbx_mr(MG, DG, RS)                                                \
        andbx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define andbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))

#define andbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define andbxZrr(RG, RS)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RS)))

#define andbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define andbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define andbxZrr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define andbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define andbxZmr(MG, DG, RS)                                                \
        andbxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annbx_ri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbx_ri(W(RG), W(IS))

#define annbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define annbx_rr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))

#define annbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))

#define annbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define annbx_mr(MG, DG, RS)                                                \
        annbx_st(W(RS), W(MG), W(DG))


#define annbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        andbxZri(W(RG), W(IS))

#if RT_BASE_COMPAT_ZFL == 0

#define annbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define annbxZrr(RG, RS)                                                    \
        EMITW(0x7C000079 | MSM(REG(RG), REG(RS), REG(RG)))

#define annbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000079 | MSM(REG(RG), TMxx,    REG(RG)))

#define annbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000079 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define annbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define annbxZrr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define annbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define annbxZmr(MG, DG, RS)                                                \
        annbxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define orrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define orrbx_rr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))

#define orrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define orrbx_mr(MG, DG, RS)                                                \
        orrbx_st(W(RS), W(MG), W(DG))


#define orrbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define orrbxZrr(RG, RS)                                                    \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), TMxx))

#define orrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000379 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define orrbxZrr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define orrbxZmr(MG, DG, RS)                                                \
        orrbxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornbx_ri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbx_ri(W(RG), W(IS))

#define ornbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define ornbx_rr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define ornbx_mr(MG, DG, RS)                                                \
        ornbx_st(W(RS), W(MG), W(DG))


#define ornbxZri(RG, IS)                                                    \
        notbx_rx(W(RG))                                                     \
        orrbxZri(W(RG), W(IS))

#define ornbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define ornbxZrr(RG, RS)                                                    \
        EMITW(0x7C000339 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000339 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000339 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define ornbxZrr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define ornbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define ornbxZmr(MG, DG, RS)                                                \
        ornbxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))

#define xorbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define xorbx_rr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))

#define xorbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define xorbx_mr(MG, DG, RS)                                                \
        xorbx_st(W(RS), W(MG), W(DG))


#define xorbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#if RT_BASE_COMPAT_ZFL == 0

#define xorbxZrr(RG, RS)                                                    \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), TMxx))

#define xorbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000279 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define xorbxZrr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define xorbxZmr(MG, DG, RS)                                                \
        xorbxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notbx_rx(RG)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RG), REG(RG), REG(RG)))

#define notbx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000F8 | MSM(TWxx,    TWxx,    TWxx))                     \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negbx_rx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))

#define negbx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))


#if RT_BASE_COMPAT_ZFL == 0

#define negbxZrx(RG)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RG), 0x00,    REG(RG)))

#define negbxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define negbxZrx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define negbxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))

#define addbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define addbx_rr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))

#define addbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define addbx_mr(MG, DG, RS)                                                \
        addbx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define addbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))

#define addbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define addbxZrr(RG, RS)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RS)))

#define addbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define addbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define addbxZrr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define addbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define addbxZmr(MG, DG, RS)                                                \
        addbxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))

#define subbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define subbx_rr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))

#define subbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define subbx_mr(MG, DG, RS)                                                \
        subbx_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_ZFL == 0

#define subbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))

#define subbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define subbxZrr(RG, RS)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RS)))

#define subbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#else /* RT_BASE_COMPAT_ZFL */

#define subbxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#define subbxZrr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subbxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x28000000 | REG(RG) << 16)              /* <- set flags (Z) */

#define subbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))  \
        EMITW(0x28000000 | TMxx << 16)                 /* <- set flags (Z) */

#endif /* RT_BASE_COMPAT_ZFL */

#define subbxZmr(MG, DG, RS)                                                \
        subbxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlbx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), Tecx))

#define shlbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shlbx_ri(RG, IS)                                                    \
        EMITW(0x54000000 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)

#define shlbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x54000000 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shlbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000030 | MSM(REG(RG), REG(RG), TMxx))

#define shlbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000030 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shlbx_mr(MG, DG, RS)                                                \
        shlbx_st(W(RS), W(MG), W(DG))


#define shlbxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), Tecx))

#define shlbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shlbxZri(RG, IS)                                                    \
        EMITW(0x54000001 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)

#define shlbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x54000001 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                           ((31-VAL(IS)) & 0x1F) << 1)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shlbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000031 | MSM(REG(RG), REG(RG), TMxx))

#define shlbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000031 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shlbxZmr(MG, DG, RS)                                                \
        shlbxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrbx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), Tecx))

#define shrbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbx_ri(RG, IS)                                                    \
        EMITW(0x5400003E | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6)

#define shrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x5400003E | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000430 | MSM(REG(RG), REG(RG), TMxx))

#define shrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000430 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbx_mr(MG, DG, RS)                                                \
        shrbx_st(W(RS), W(MG), W(DG))


#define shrbxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), Tecx))

#define shrbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbxZri(RG, IS)                                                    \
        EMITW(0x5400003F | MSM(REG(RG), REG(RG), (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6)

#define shrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x5400003F | MSM(TMxx,    TMxx,    (32-VAL(IS)) & 0x1F) |     \
                                                     (VAL(IS) & 0x1F) << 6) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000431 | MSM(REG(RG), REG(RG), TMxx))

#define shrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000431 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbxZmr(MG, DG, RS)                                                \
        shrbxZst(W(RS), W(MG), W(DG))


#define shrbn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), Tecx))

#define shrbn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000630 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbn_ri(RG, IS)                                                    \
        EMITW(0x7C000670 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F))

#define shrbn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000670 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F))           \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrbn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000630 | MSM(REG(RG), REG(RG), TMxx))

#define shrbn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000630 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbn_mr(MG, DG, RS)                                                \
        shrbn_st(W(RS), W(MG), W(DG))


#define shrbnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), Tecx))

#define shrbnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000631 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbnZri(RG, IS)                                                    \
        EMITW(0x7C000671 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F))

#define shrbnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000671 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F))           \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrbnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000631 | MSM(REG(RG), REG(RG), TMxx))

#define shrbnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), PB(DG)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000631 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), OB(DG)))

#define shrbnZmr(MG, DG, RS)                                                \
        shrbnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulbx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TIxx))

#define mulbx_rr(RG, RS)                                                    \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C0001D6 | MRM(REG(RG), REG(RG), TMxx))


#define mulbx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x5400003E | MSM(Tedx,    Teax,    24) | 8 << 6)

#define mulbx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x5400003E | MSM(Tedx,    Teax,    24) | 8 << 6)


#define mulbn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0x7C000670 | MSM(Tedx,    Teax,    8))

#define mulbn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C0001D6 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0x7C000670 | MSM(Tedx,    Teax,    8))


#define mulbp_xr(RS)     /* Reax is in/out, prepares Redx for divbn_x* */   \
        mulbx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulbp_xm(MS, DS) /* Reax is in/out, prepares Redx for divbn_x* */   \
        mulbx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divbx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), TIxx))

#define divbx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), REG(RS)))

#define divbx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000396 | MTM(REG(RG), REG(RG), TMxx))


#define divbn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TIxx))

#define divbn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), REG(RS)))

#define divbn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C0003D6 | MTM(REG(RG), REG(RG), TMxx))


#define prebx_xx()          /* to be placed immediately prior divbx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prebn_xx()          /* to be placed immediately prior divbn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divbx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    REG(RS)))

#define divbx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000396 | MTM(Teax,    Teax,    TMxx))


#define divbn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    REG(RS)))

#define divbn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C0003D6 | MTM(Teax,    Teax,    TMxx))


#define divbp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divbn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divbp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divbn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#if RT_BASE_COMPAT_REM == 0

#define rembx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movbx_rr(Redx, W(RG))                                               \
        divbx_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define rembx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movbx_rr(Redx, W(RG))                                               \
        divbx_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define rembx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movbx_rr(Redx, W(RG))                                               \
        divbx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define rembn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movbx_rr(Redx, W(RG))                                               \
        divbn_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define rembn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movbx_rr(Redx, W(RG))                                               \
        divbn_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define rembn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movbx_rr(Redx, W(RG))                                               \
        divbn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D6 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define rembx_xx()          /* to be placed immediately prior divbx_x* */   \
        movbx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembx_xr(RS)        /* to be placed immediately after divbx_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define rembx_xm(MS, DS)    /* to be placed immediately after divbx_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define rembn_xx()          /* to be placed immediately prior divbn_x* */   \
        movbx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembn_xr(RS)        /* to be placed immediately after divbn_xr */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define rembn_xm(MS, DS)    /* to be placed immediately after divbn_xm */   \
        EMITW(0x7C0001D6 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#else /* RT_BASE_COMPAT_REM != 0 */

#define rembx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), TIxx))

#define rembx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), REG(RS)))

#define rembx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000216 | MTM(REG(RG), REG(RG), TMxx))


#define rembn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), TIxx))

#define rembn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), REG(RS)))

#define rembn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000774 | MSM(TMxx,    TMxx,    0x00))                     \
        EMITW(0x7C000616 | MTM(REG(RG), REG(RG), TMxx))


#define rembx_xx()          /* to be placed immediately prior divbx_x* */   \
        movbx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembx_xr(RS)        /* to be placed immediately after divbx_xr */   \
        EMITW(0x7C000216 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define rembx_xm(MS, DS)    /* to be placed immediately after divbx_xm */   \
        EMITW(0x7C000216 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define rembn_xx()          /* to be placed immediately prior divbn_x* */   \
        movbx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define rembn_xr(RS)        /* to be placed immediately after divbn_xr */   \
        EMITW(0x7C000616 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define rembn_xm(MS, DS)    /* to be placed immediately after divbn_xm */   \
        EMITW(0x7C000616 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#endif /* RT_BASE_COMPAT_REM != 0 */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

#define add_n   AM3
#define sub_n   AM4

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjbx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, h, Zrx)                                              \
        CMJ(cc, lb)

#define arjbx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, h, Zmx)                                       \
        CMJ(cc, lb)

#define arjbx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, h, Zri)                                       \
        CMJ(cc, lb)

#define arjbx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, h, Zmi)                                \
        CMJ(cc, lb)

#define arjbx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, h, Zrr)                                       \
        CMJ(cc, lb)

#define arjbx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, h, Zld)                                \
        CMJ(cc, lb)

#define arjbx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, h, Zst)                                \
        CMJ(cc, lb)

#define arjbx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjbx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjbx_rz(RS, cc, lb)                                                \
        cmjbx_ri(W(RS), IC(0), cc, lb)

#define cmjbx_mz(MS, DS, cc, lb)                                            \
        cmjbx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjbx_ri(RS, IT, cc, lb)                                            \
        CWI(cc, MOD(RS), REG(RS), W(IT), lb)

#define cmjbx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        CWI(cc, %%r24,   TMxx,    W(IT), lb)

#define cmjbx_rr(RS, RT, cc, lb)                                            \
        CWR(cc, MOD(RS), MOD(RT), lb)

#define cmjbx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), PB(DT)))  \
        CWR(cc, MOD(RS), %%r24,   lb)

#define cmjbx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        CWR(cc, %%r24,   MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpbx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpbx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))

#define cmpbx_rr(RS, RT)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpbx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PB(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpbx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PB(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_PHB_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

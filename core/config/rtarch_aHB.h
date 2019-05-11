/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_AHB_H
#define RT_RTARCH_AHB_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_aHB.h: Implementation of AArch64 half+byte BASE instructions.
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
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x78000000 | MDM(TIxx,    MOD(MD), VHL(DD), B1(DD), P1(DD)))

#define movhx_rr(RD, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RD), TZxx,    REG(RS)))

#define movhx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(REG(RD), MOD(MS), VHL(DS), B1(DS), P1(DS)))

#define movhn_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(REG(RD), MOD(MS), VHL(DS), B1(DS), P1(DS)))

#define movhx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x78000000 | MDM(REG(RS), MOD(MD), VHL(DD), B1(DD), P1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*x), yes (*z) */

#define andhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhx_rr(RG, RS)                                                    \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhx_mr(MG, DG, RS)                                                \
        andhx_st(W(RS), W(MG), W(DG))


#define andhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x60000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x60000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhz_rr(RG, RS)                                                    \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define andhz_mr(MG, DG, RS)                                                \
        andhz_st(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhx_ri(W(RG), W(IS))

#define annhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define annhx_rr(RG, RS)                                                    \
        EMITW(0x0A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define annhx_mr(MG, DG, RS)                                                \
        annhx_st(W(RS), W(MG), W(DG))


#define annhz_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhz_ri(W(RG), W(IS))

#define annhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x60000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define annhz_rr(RG, RS)                                                    \
        EMITW(0x6A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define annhz_mr(MG, DG, RS)                                                \
        annhz_st(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

#define orrhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define orrhx_rr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define orrhx_mr(MG, DG, RS)                                                \
        orrhx_st(W(RS), W(MG), W(DG))


#define orrhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x20000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrhz_rr(RG, RS)                                                    \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrhz_mr(MG, DG, RS)                                                \
        orrhz_st(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornhx_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhx_ri(W(RG), W(IS))

#define ornhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define ornhx_rr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define ornhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define ornhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define ornhx_mr(MG, DG, RS)                                                \
        ornhx_st(W(RS), W(MG), W(DG))


#define ornhz_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhz_ri(W(RG), W(IS))

#define ornhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x20000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornhz_rr(RG, RS)                                                    \
        EMITW(0x2A200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2A200000 | MRM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornhz_mr(MG, DG, RS)                                                \
        ornhz_st(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

#define xorhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define xorhx_rr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define xorhx_mr(MG, DG, RS)                                                \
        xorhx_st(W(RS), W(MG), W(DG))


#define xorhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x40000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x40000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorhz_rr(RG, RS)                                                    \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4A000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorhz_mr(MG, DG, RS)                                                \
        xorhz_st(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define nothx_rx(RG)                                                        \
        EMITW(0x2A200000 | MRM(REG(RG), TZxx,    REG(RG)))

#define nothx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2A200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define neghx_rx(RG)                                                        \
        EMITW(0x4B000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define neghx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))


#define neghz_rx(RG)                                                        \
        EMITW(0x6B000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define neghz_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

#define addhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x01000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x01000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define addhx_rr(RG, RS)                                                    \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addhn_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x0B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x0B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#define addhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x21000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x21000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define addhz_rr(RG, RS)                                                    \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x2B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x2B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define addhz_mr(MG, DG, RS)                                                \
        addhz_st(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

#define subhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x41000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x41000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define subhx_rr(RG, RS)                                                    \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subhn_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x4B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x4B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#define subhz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x61000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x61000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define subhz_rr(RG, RS)                                                    \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subhz_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6B000000 | MRM(REG(RG), REG(RG), TMxx))

#define subhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x6B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define subhz_mr(MG, DG, RS)                                                \
        subhz_st(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))

#define shlhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shlhx_ri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)

#define shlhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))

#define shlhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shlhx_mr(MG, DG, RS)                                                \
        shlhx_st(W(RS), W(MG), W(DG))


#define shlhz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlhz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlhz_ri(RG, IS)                                                    \
        EMITW(0x53000000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53000000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x1F)<<16 | (31-(VAL(IS)&0x1F))<<10)                      \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlhz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlhz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlhz_mr(MG, DG, RS)                                                \
        shlhz_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))

#define shrhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhx_ri(RG, IS)                                                    \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhx_mr(MG, DG, RS)                                                \
        shrhx_st(W(RS), W(MG), W(DG))


#define shrhz_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrhz_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrhz_ri(RG, IS)                                                    \
        EMITW(0x53007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrhz_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x53007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrhz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrhz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02400 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0x71000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrhz_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x71000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrhz_mr(MG, DG, RS)                                                \
        shrhz_st(W(RS), W(MG), W(DG))


#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), Tecx))

#define shrhn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhn_ri(RG, IS)                                                    \
        EMITW(0x13007C00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x1F)<<16)

#define shrhn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x13007C00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x1F)<<16) \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrhn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrhn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))  \
        EMITW(0x1AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x78000000 | MDM(TMxx,    MOD(MG), VHL(DG), B1(DG), P1(DG)))

#define shrhn_mr(MG, DG, RS)                                                \
        shrhn_st(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulhx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TIxx))

#define mulhx_rr(RG, RS)                                                    \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))

#define mulhn_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9BA07C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    REG(RS)))                  \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9B207C00 | MRM(Teax,    Teax,    TMxx))                     \
        EMITW(0xD360FC00 | MRM(Tedx,    Teax,    0x00))


#define mulhp_xr(RS)     /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulhp_xm(MS, DS) /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divhx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divhx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), REG(RS)))

#define divhx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divhn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divhn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define divhn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prehx_xx()          /* to be placed immediately prior divhx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prehn_xx()          /* to be placed immediately prior divhn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    REG(RS)))

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00800 | MRM(Teax,    Teax,    TMxx))


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    REG(RS)))

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x1AC00C00 | MRM(Teax,    Teax,    TMxx))


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
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remhx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhx_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remhx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remhn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhn_ri(W(RG), W(IS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remhn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhn_rr(W(RG), W(RS))                                              \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remhn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movhx_rr(Redx, W(RG))                                               \
        divhn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x1B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remhx_xx()          /* to be placed immediately prior divhx_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remhn_xx()          /* to be placed immediately prior divhn_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remhn_xr(RS)        /* to be placed immediately after divhn_xr */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remhn_xm(MS, DS)    /* to be placed immediately after divhn_xm */   \
        EMITW(0x1B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

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
        cmphx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjhx_mi(MS, DS, IT, cc, lb)                                        \
        cmphx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjnx_mi(MS, DS, IT, cc, lb)                                        \
        cmpnx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjhx_rr(RS, RT, cc, lb)                                            \
        cmphx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjhx_rm(RS, MT, DT, cc, lb)                                        \
        cmphx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhn_rm(RS, MT, DT, cc, lb)                                        \
        cmphn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjhx_mr(MS, DS, RT, cc, lb)                                        \
        cmphx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjhn_mr(MS, DS, RT, cc, lb)                                        \
        cmphn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmphx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IT))   \
        EMITW(0x61000000 | MIM(TZxx,    REG(RS), VAL(IT), T1(IT), M1(IT)))

#define cmphx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), C1(DS), G1(IT))   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x61000000 | MIM(TZxx,    TMxx,    VAL(IT), T1(IT), M1(IT)))

#define cmphn_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), C1(DS), G1(IT))   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x61000000 | MIM(TZxx,    TMxx,    VAL(IT), T1(IT), M1(IT)))

#define cmphx_rr(RS, RT)                                                    \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RS), REG(RT)))

#define cmphx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RS), TMxx))

#define cmphn_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MT), VHL(DT), B1(DT), P1(DT)))  \
        EMITW(0x6B000000 | MRM(TZxx,    REG(RS), TMxx))

#define cmphx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78400000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    REG(RT)))

#define cmphn_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x78800000 | MDM(TMxx,    MOD(MS), VHL(DS), B1(DS), P1(DS)))  \
        EMITW(0x6B000000 | MRM(TZxx,    TMxx,    REG(RT)))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_AHB_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

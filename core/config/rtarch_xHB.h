/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_XHB_H
#define RT_RTARCH_XHB_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_xHB.h: Implementation of x86_64 half+byte BASE instructions.
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
    ESC REX(0,       RXB(RD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RD), REG(RD))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movhx_mi(MD, DD, IS)                                                \
ADR ESC REX(0,       RXB(MD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(MD), REG(MD))   /* truncate IC with TYP below */   \
        AUX(SIB(MD), CMD(DD), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movhx_rr(RD, RS)                                                    \
        REX(RXB(RD), RXB(RS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movhx_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xB7)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movhn_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x0F) EMITB(0xBF)                       \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movhx_st(RS, MD, DD)                                                \
ADR ESC REX(RXB(RS), RXB(MD)) EMITB(0x89)                                   \
        MRM(REG(RS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* and (G = G & S)
 * set-flags: undefined (*x), yes (*z) */

#define andhx_ri(RG, IS)                                                    \
        andhz_ri(W(RG), W(IS))

#define andhx_mi(MG, DG, IS)                                                \
        andhz_mi(W(MG), W(DG), W(IS))

#define andhx_rr(RG, RS)                                                    \
        andhz_rr(W(RG), W(RS))

#define andhx_ld(RG, MS, DS)                                                \
        andhz_ld(W(RG), W(MS), W(DS))

#define andhx_st(RS, MG, DG)                                                \
        andhz_st(W(RS), W(MG), W(DG))

#define andhx_mr(MG, DG, RS)                                                \
        andhx_st(W(RS), W(MG), W(DG))


#define andhz_ri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x04,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define andhz_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x04,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define andhz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define andhz_ld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andhz_st(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x21)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define andhz_mr(MG, DG, RS)                                                \
        andhz_st(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annhx_ri(RG, IS)                                                    \
        annhz_ri(W(RG), W(IS))

#define annhx_mi(MG, DG, IS)                                                \
        annhz_mi(W(MG), W(DG), W(IS))

#define annhx_rr(RG, RS)                                                    \
        annhz_rr(W(RG), W(RS))

#define annhx_ld(RG, MS, DS)                                                \
        annhz_ld(W(RG), W(MS), W(DS))

#define annhx_st(RS, MG, DG)                                                \
        annhz_st(W(RS), W(MG), W(DG))

#define annhx_mr(MG, DG, RS)                                                \
        annhx_st(W(RS), W(MG), W(DG))


#define annhz_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhz_ri(W(RG), W(IS))

#define annhz_mi(MG, DG, IS)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        andhz_mi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define annhz_rr(RG, RS)                                                    \
        nothx_rx(W(RG))                                                     \
        andhz_rr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define annhz_rr(RG, RS)                                                    \
        VEX(RXB(RG), RXB(RS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RS), REG(RS))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define annhz_ld(RG, MS, DS)                                                \
        nothx_rx(W(RG))                                                     \
        andhz_ld(W(RG), W(MS), W(DS))

#define annhz_st(RS, MG, DG)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        andhz_st(W(RS), W(MG), W(DG))

#define annhz_mr(MG, DG, RS)                                                \
        annhz_st(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

#define orrhx_ri(RG, IS)                                                    \
        orrhz_ri(W(RG), W(IS))

#define orrhx_mi(MG, DG, IS)                                                \
        orrhz_mi(W(MG), W(DG), W(IS))

#define orrhx_rr(RG, RS)                                                    \
        orrhz_rr(W(RG), W(RS))

#define orrhx_ld(RG, MS, DS)                                                \
        orrhz_ld(W(RG), W(MS), W(DS))

#define orrhx_st(RS, MG, DG)                                                \
        orrhz_st(W(RS), W(MG), W(DG))

#define orrhx_mr(MG, DG, RS)                                                \
        orrhx_st(W(RS), W(MG), W(DG))


#define orrhz_ri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x01,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define orrhz_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x01,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define orrhz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define orrhz_ld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrhz_st(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x09)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define orrhz_mr(MG, DG, RS)                                                \
        orrhz_st(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornhx_ri(RG, IS)                                                    \
        ornhz_ri(W(RG), W(IS))

#define ornhx_mi(MG, DG, IS)                                                \
        ornhz_mi(W(MG), W(DG), W(IS))

#define ornhx_rr(RG, RS)                                                    \
        ornhz_rr(W(RG), W(RS))

#define ornhx_ld(RG, MS, DS)                                                \
        ornhz_ld(W(RG), W(MS), W(DS))

#define ornhx_st(RS, MG, DG)                                                \
        ornhz_st(W(RS), W(MG), W(DG))

#define ornhx_mr(MG, DG, RS)                                                \
        ornhx_st(W(RS), W(MG), W(DG))


#define ornhz_ri(RG, IS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhz_ri(W(RG), W(IS))

#define ornhz_mi(MG, DG, IS)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        orrhz_mi(W(MG), W(DG), W(IS))

#define ornhz_rr(RG, RS)                                                    \
        nothx_rx(W(RG))                                                     \
        orrhz_rr(W(RG), W(RS))

#define ornhz_ld(RG, MS, DS)                                                \
        nothx_rx(W(RG))                                                     \
        orrhz_ld(W(RG), W(MS), W(DS))

#define ornhz_st(RS, MG, DG)                                                \
        nothx_mx(W(MG), W(DG))                                              \
        orrhz_st(W(RS), W(MG), W(DG))

#define ornhz_mr(MG, DG, RS)                                                \
        ornhz_st(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

#define xorhx_ri(RG, IS)                                                    \
        xorhz_ri(W(RG), W(IS))

#define xorhx_mi(MG, DG, IS)                                                \
        xorhz_mi(W(MG), W(DG), W(IS))

#define xorhx_rr(RG, RS)                                                    \
        xorhz_rr(W(RG), W(RS))

#define xorhx_ld(RG, MS, DS)                                                \
        xorhz_ld(W(RG), W(MS), W(DS))

#define xorhx_st(RS, MG, DG)                                                \
        xorhz_st(W(RS), W(MG), W(DG))

#define xorhx_mr(MG, DG, RS)                                                \
        xorhx_st(W(RS), W(MG), W(DG))


#define xorhz_ri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x06,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define xorhz_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x06,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define xorhz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define xorhz_ld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorhz_st(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x31)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define xorhz_mr(MG, DG, RS)                                                \
        xorhz_st(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define nothx_rx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RG), REG(RG))

#define nothx_mx(MG, DG)                                                    \
ADR ESC REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define neghx_rx(RG)                                                        \
        neghz_rx(W(RG))

#define neghx_mx(MG, DG)                                                    \
        neghz_mx(W(MG), W(DG))


#define neghz_rx(RG)                                                        \
        REX(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RG), REG(RG))

#define neghz_mx(MG, DG)                                                    \
ADR ESC REX(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

#define addhx_ri(RG, IS)                                                    \
        addhz_ri(W(RG), W(IS))

#define addhx_mi(MG, DG, IS)                                                \
        addhz_mi(W(MG), W(DG), W(IS))

#define addhx_rr(RG, RS)                                                    \
        addhz_rr(W(RG), W(RS))

#define addhx_ld(RG, MS, DS)                                                \
        addhz_ld(W(RG), W(MS), W(DS))

#define addhx_st(RS, MG, DG)                                                \
        addhz_st(W(RS), W(MG), W(DG))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#define addhz_ri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x00,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define addhz_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x00,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define addhz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define addhz_ld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addhz_st(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x01)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define addhz_mr(MG, DG, RS)                                                \
        addhz_st(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

#define subhx_ri(RG, IS)                                                    \
        subhz_ri(W(RG), W(IS))

#define subhx_mi(MG, DG, IS)                                                \
        subhz_mi(W(MG), W(DG), W(IS))

#define subhx_rr(RG, RS)                                                    \
        subhz_rr(W(RG), W(RS))

#define subhx_ld(RG, MS, DS)                                                \
        subhz_ld(W(RG), W(MS), W(DS))

#define subhx_st(RS, MG, DG)                                                \
        subhz_st(W(RS), W(MG), W(DG))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#define subhz_ri(RG, IS)                                                    \
    ESC REX(0,       RXB(RG)) EMITB(0x81)                                   \
        MRM(0x05,    MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define subhz_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0x81)                                   \
        MRM(0x05,    MOD(MG), REG(MG))   /* truncate IC with TYP below */   \
        AUX(SIB(MG), CMD(DG), EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define subhz_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define subhz_ld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subhz_st(RS, MG, DG)                                                \
ADR ESC REX(RXB(RS), RXB(MG)) EMITB(0x29)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define subhz_mr(MG, DG, RS)                                                \
        subhz_st(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        shlhz_rx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlhz_mx(W(MG), W(DG))

#define shlhx_ri(RG, IS)                                                    \
        shlhz_ri(W(RG), W(IS))

#define shlhx_mi(MG, DG, IS)                                                \
        shlhz_mi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlhz_rr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlhz_ld(W(RG), W(MS), W(DS))

#define shlhx_st(RS, MG, DG)                                                \
        shlhz_st(W(RS), W(MG), W(DG))

#define shlhx_mr(MG, DG, RS)                                                \
        shlhx_st(W(RS), W(MG), W(DG))


#define shlhz_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \

#define shlhz_mx(MG, DG)                 /* reads Recx for shift count */   \
ADR ESC REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shlhz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shlhz_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shlhz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shlhz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlhz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx, W(MS), W(DS))                                        \
        shlhz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlhz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shlhz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shlhz_mr(MG, DG, RS)                                                \
        shlhz_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z)
 * for maximum compatibility: shift count must be modulo elem-size */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        shrhz_rx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrhz_mx(W(MG), W(DG))

#define shrhx_ri(RG, IS)                                                    \
        shrhz_ri(W(RG), W(IS))

#define shrhx_mi(MG, DG, IS)                                                \
        shrhz_mi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrhz_rr(W(RG), W(RS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrhz_ld(W(RG), W(MS), W(DS))

#define shrhx_st(RS, MG, DG)                                                \
        shrhz_st(W(RS), W(MG), W(DG))

#define shrhx_mr(MG, DG, RS)                                                \
        shrhx_st(W(RS), W(MG), W(DG))


#define shrhz_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \

#define shrhz_mx(MG, DG)                 /* reads Recx for shift count */   \
ADR ESC REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrhz_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrhz_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#define shrhz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx, W(MS), W(DS))                                        \
        shrhz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrhz_mr(MG, DG, RS)                                                \
        shrhz_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
        REX(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
        VEX(RXB(RG), RXB(RG),    0x01, 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhn_mx(MG, DG)                 /* reads Recx for shift count */   \
ADR ESC REX(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrhn_ri(RG, IS)                                                    \
        REX(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS)))

#define shrhn_mi(MG, DG, IS)                                                \
ADR ESC REX(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS)))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhn_rx(W(RG))                                                     \
        stack_ld(Recx)

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEX(RXB(RG), RXB(RG), REN(RS), 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrhn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movhx_ld(Recx, W(MS), W(DS))                                        \
        shrhn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrhn_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movhx_rr(Recx, W(RS))                                               \
        shrhn_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrhn_mr(MG, DG, RS)                                                \
        shrhn_st(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulhx_ri(RG, IS)                                                    \
    ESC REX(RXB(RG), RXB(RG)) EMITB(0x69)                                   \
        MRM(REG(RG), MOD(RG), REG(RG))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITH(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define mulhx_rr(RG, RS)                                                    \
        REX(RXB(RG), RXB(RS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RS), REG(RS))

#define mulhx_ld(RG, MS, DS)                                                \
ADR ESC REX(RXB(RG), RXB(MS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
ADR ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RS), REG(RS))                                      \

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
ADR ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RS), REG(RS))                                      \

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulhp_xr(RS)     /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhn_xr(W(RS))       /* product must not exceed operands size */

#define mulhp_xm(MS, DS) /* Reax is in/out, prepares Redx for divhn_x* */   \
        mulhn_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divhx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divhn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divhn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movhx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prehx_xx()          /* to be placed immediately prior divhx_x* */   \
        movhx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prehn_xx()          /* to be placed immediately prior divhn_x* */   \
        movhx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrhn_ri(Redx, IC(31))


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
ADR ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
ADR ESC REX(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
ADR ESC REX(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


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
        stack_st(Reax)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehx_xx()                                                          \
        divhx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remhn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remhn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movhx_rr(Reax, W(RG))                                               \
        prehn_xx()                                                          \
        divhn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movhx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remhx_xx()          /* to be placed immediately prior divhx_x* */   \
                                     /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remhn_xx()          /* to be placed immediately prior divhn_x* */   \
                                     /* to prepare for rem calculation */

#define remhn_xr(RS)        /* to be placed immediately after divhn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remhn_xm(MS, DS)    /* to be placed immediately after divhn_xm */   \
                                     /* to produce remainder Redx<-rem */

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

#define cmjhn_mi(MS, DS, IT, cc, lb)                                        \
        cmphn_mi(W(MS), W(DS), W(IT))                                       \
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
        REX(0,       RXB(RS)) EMITB(0x81)                                   \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IT))

#define cmphx_mi(MS, DS, IT)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x81)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), CMD(IT))

#define cmphn_mi(MS, DS, IT)                                                \
ADR ESC REX(0,       RXB(MS)) EMITB(0x81)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), CMD(IT))

#define cmphx_rr(RS, RT)                                                    \
        REX(RXB(RS), RXB(RT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(RT), REG(RT))

#define cmphx_rm(RS, MT, DT)                                                \
ADR ESC REX(RXB(RS), RXB(MT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmphn_rm(RS, MT, DT)                                                \
ADR ESC REX(RXB(RS), RXB(MT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmphx_mr(MS, DS, RT)                                                \
ADR ESC REX(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define cmphn_mr(MS, DS, RT)                                                \
ADR ESC REX(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_XHB_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

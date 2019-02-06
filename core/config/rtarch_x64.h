/******************************************************************************/
/* Copyright (c) 2013-2019 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_X64_H
#define RT_RTARCH_X64_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_x64.h: Implementation of x86_64:x64 BASE instructions.
 *
 * This file is a part of the unified SIMD assembler framework (rtarch.h)
 * designed to be compatible with different processor architectures,
 * while maintaining strictly defined common API.
 *
 * Recommended naming scheme for instructions:
 *
 * cmdxx_ri - applies [cmd] to [r]egister from [i]mmediate
 * cmdxx_mi - applies [cmd] to [m]emory   from [i]mmediate
 * cmdxx_rz - applies [cmd] to [r]egister from [z]ero-arg
 * cmdxx_mz - applies [cmd] to [m]emory   from [z]ero-arg
 *
 * cmdxx_rm - applies [cmd] to [r]egister from [m]emory
 * cmdxx_ld - applies [cmd] as above
 * cmdxx_mr - applies [cmd] to [m]emory   from [r]egister
 * cmdxx_st - applies [cmd] as above (arg list as cmdxx_ld)
 *
 * cmdxx_rr - applies [cmd] to [r]egister from [r]egister
 * cmdxx_mm - applies [cmd] to [m]emory   from [m]emory
 * cmdxx_rx - applies [cmd] to [r]egister (one-operand cmd)
 * cmdxx_mx - applies [cmd] to [m]emory   (one-operand cmd)
 *
 * cmdxx_rx - applies [cmd] to [r]egister from x-register
 * cmdxx_mx - applies [cmd] to [m]emory   from x-register
 * cmdxx_xr - applies [cmd] to x-register from [r]egister
 * cmdxx_xm - applies [cmd] to x-register from [m]emory
 *
 * cmdxx_rl - applies [cmd] to [r]egister from [l]abel
 * cmdxx_xl - applies [cmd] to x-register from [l]abel
 * cmdxx_lb - applies [cmd] as above
 * label_ld - applies [adr] as above
 *
 * stack_st - applies [mov] to stack from register (push)
 * stack_ld - applies [mov] to register from stack (pop)
 * stack_sa - applies [mov] to stack from all registers
 * stack_la - applies [mov] to all registers from stack
 *
 * cmdw*_** - applies [cmd] to 32-bit BASE register/memory/immediate args
 * cmdx*_** - applies [cmd] to A-size BASE register/memory/immediate args
 * cmdy*_** - applies [cmd] to L-size BASE register/memory/immediate args
 * cmdz*_** - applies [cmd] to 64-bit BASE register/memory/immediate args
 *
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
 * cmd*p_** - applies [cmd] to   signed integer args, [p] - part-range
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
 *
 * Alphabetical view of current/future instruction namespaces is in rtzero.h.
 * Configurable BASE/SIMD subsets (cmdx*, cmdy*, cmdp*) are defined in rtconf.h.
 * Mixing of 64/32-bit fields in backend structures may lead to misalignment
 * of 64-bit fields to 4-byte boundary, which is not supported on some targets.
 * Place fields carefully to ensure natural alignment for all data types.
 * Note that within cmdx*_** subset most of the instructions follow in-heap
 * address size (RT_ADDRESS or A) and only label_ld/st, jmpxx_xr/xm follow
 * pointer size (RT_POINTER or P) as code/data/stack segments are fixed.
 * Stack ops always work with full registers regardless of the mode chosen.
 *
 * 32-bit and 64-bit BASE subsets are not easily compatible on all targets,
 * thus any register modified with 32-bit op cannot be used in 64-bit subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subsets across all targets.
 * Registers written with 64-bit op aren't always compatible with 32-bit either,
 * as m64 requires the upper half to be all 0s or all 1s for m32 arithmetic.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subsets,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (32-bit, 64-bit or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtbase.h.
 *
 * Setting-flags instruction naming scheme may change again in the future for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and POWER use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
 *
 * Argument x-register (implied) is fixed by the implementation.
 * Some formal definitions are not given below to encourage
 * use of friendly aliases for better code readability.
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

#include "rtarch_x32.h"

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movzx_ri(RD, IS)                                                    \
        REW(0,       RXB(RD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RD), REG(RD))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movzx_mi(MD, DD, IS)                                                \
    ADR REW(0,       RXB(MD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(MD), REG(MD))   /* truncate IC with TYP below */   \
        AUX(SIB(MD), CMD(DD), EMITW(VAL(IS) & ((TYP(IS) << 6) - 1)))

#define movzx_rr(RD, RS)                                                    \
        REW(RXB(RD), RXB(RS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movzx_ld(RD, MS, DS)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define movzx_st(RS, MD, DD)                                                \
    ADR REW(RXB(RS), RXB(MD)) EMITB(0x89)                                   \
        MRM(REG(RS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define movzx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        REW(0,       RXB(RD)) EMITB(0xB8 + REG(RD))                         \
        AUX(EMPTY,   EMITW(VAL(IS)), EMITW(VAL(IT)))

#define movzx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        REW(0,          0x01) EMITB(0xB8 + 0x07)                            \
        AUX(EMPTY,   EMITW(VAL(IS)), EMITW(VAL(IT)))                        \
    ADR REW(0x01,    RXB(MD)) EMITB(0x89)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

/* and (G = G & S)
 * set-flags: undefined (*x), yes (*z) */

#define andzx_ri(RG, IS)                                                    \
        andzz_ri(W(RG), W(IS))

#define andzx_mi(MG, DG, IS)                                                \
        andzz_mi(W(MG), W(DG), W(IS))

#define andzx_rr(RG, RS)                                                    \
        andzz_rr(W(RG), W(RS))

#define andzx_ld(RG, MS, DS)                                                \
        andzz_ld(W(RG), W(MS), W(DS))

#define andzx_st(RS, MG, DG)                                                \
        andzz_st(W(RS), W(MG), W(DG))

#define andzx_mr(MG, DG, RS)                                                \
        andzx_st(W(RS), W(MG), W(DG))


#define andzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define andzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define andzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define andzz_ld(RG, MS, DS)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define andzz_st(RS, MG, DG)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x21)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define andzz_mr(MG, DG, RS)                                                \
        andzz_st(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annzx_ri(RG, IS)                                                    \
        annzz_ri(W(RG), W(IS))

#define annzx_mi(MG, DG, IS)                                                \
        annzz_mi(W(MG), W(DG), W(IS))

#define annzx_rr(RG, RS)                                                    \
        annzz_rr(W(RG), W(RS))

#define annzx_ld(RG, MS, DS)                                                \
        annzz_ld(W(RG), W(MS), W(DS))

#define annzx_st(RS, MG, DG)                                                \
        annzz_st(W(RS), W(MG), W(DG))

#define annzx_mr(MG, DG, RS)                                                \
        annzx_st(W(RS), W(MG), W(DG))


#define annzz_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzz_ri(W(RG), W(IS))

#define annzz_mi(MG, DG, IS)                                                \
        notzx_mx(W(MG), W(DG))                                              \
        andzz_mi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define annzz_rr(RG, RS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzz_rr(W(RG), W(RS))

#define annzz_ld(RG, MS, DS)                                                \
        notzx_rx(W(RG))                                                     \
        andzz_ld(W(RG), W(MS), W(DS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define annzz_rr(RG, RS)                                                    \
        VEW(RXB(RG), RXB(RS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(RS), REG(RS))

#define annzz_ld(RG, MS, DS)                                                \
        VEW(RXB(RG), RXB(MS), REN(RG), 0, 0, 2) EMITB(0xF2)                 \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define annzz_st(RS, MG, DG)                                                \
        notzx_mx(W(MG), W(DG))                                              \
        andzz_st(W(RS), W(MG), W(DG))

#define annzz_mr(MG, DG, RS)                                                \
        annzz_st(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*x), yes (*z) */

#define orrzx_ri(RG, IS)                                                    \
        orrzz_ri(W(RG), W(IS))

#define orrzx_mi(MG, DG, IS)                                                \
        orrzz_mi(W(MG), W(DG), W(IS))

#define orrzx_rr(RG, RS)                                                    \
        orrzz_rr(W(RG), W(RS))

#define orrzx_ld(RG, MS, DS)                                                \
        orrzz_ld(W(RG), W(MS), W(DS))

#define orrzx_st(RS, MG, DG)                                                \
        orrzz_st(W(RS), W(MG), W(DG))

#define orrzx_mr(MG, DG, RS)                                                \
        orrzx_st(W(RS), W(MG), W(DG))


#define orrzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x01,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define orrzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define orrzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define orrzz_ld(RG, MS, DS)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define orrzz_st(RS, MG, DG)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x09)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define orrzz_mr(MG, DG, RS)                                                \
        orrzz_st(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornzx_ri(RG, IS)                                                    \
        ornzz_ri(W(RG), W(IS))

#define ornzx_mi(MG, DG, IS)                                                \
        ornzz_mi(W(MG), W(DG), W(IS))

#define ornzx_rr(RG, RS)                                                    \
        ornzz_rr(W(RG), W(RS))

#define ornzx_ld(RG, MS, DS)                                                \
        ornzz_ld(W(RG), W(MS), W(DS))

#define ornzx_st(RS, MG, DG)                                                \
        ornzz_st(W(RS), W(MG), W(DG))

#define ornzx_mr(MG, DG, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DG))


#define ornzz_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzz_ri(W(RG), W(IS))

#define ornzz_mi(MG, DG, IS)                                                \
        notzx_mx(W(MG), W(DG))                                              \
        orrzz_mi(W(MG), W(DG), W(IS))

#define ornzz_rr(RG, RS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzz_rr(W(RG), W(RS))

#define ornzz_ld(RG, MS, DS)                                                \
        notzx_rx(W(RG))                                                     \
        orrzz_ld(W(RG), W(MS), W(DS))

#define ornzz_st(RS, MG, DG)                                                \
        notzx_mx(W(MG), W(DG))                                              \
        orrzz_st(W(RS), W(MG), W(DG))

#define ornzz_mr(MG, DG, RS)                                                \
        ornzz_st(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*x), yes (*z) */

#define xorzx_ri(RG, IS)                                                    \
        xorzz_ri(W(RG), W(IS))

#define xorzx_mi(MG, DG, IS)                                                \
        xorzz_mi(W(MG), W(DG), W(IS))

#define xorzx_rr(RG, RS)                                                    \
        xorzz_rr(W(RG), W(RS))

#define xorzx_ld(RG, MS, DS)                                                \
        xorzz_ld(W(RG), W(MS), W(DS))

#define xorzx_st(RS, MG, DG)                                                \
        xorzz_st(W(RS), W(MG), W(DG))

#define xorzx_mr(MG, DG, RS)                                                \
        xorzx_st(W(RS), W(MG), W(DG))


#define xorzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x06,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define xorzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x06,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define xorzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define xorzz_ld(RG, MS, DS)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define xorzz_st(RS, MG, DG)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x31)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define xorzz_mr(MG, DG, RS)                                                \
        xorzz_st(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        REW(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RG), REG(RG))

#define notzx_mx(MG, DG)                                                    \
    ADR REW(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* neg (G = -G)
 * set-flags: undefined (*x), yes (*z) */

#define negzx_rx(RG)                                                        \
        negzz_rx(W(RG))

#define negzx_mx(MG, DG)                                                    \
        negzz_mx(W(MG), W(DG))


#define negzz_rx(RG)                                                        \
        REW(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RG), REG(RG))

#define negzz_mx(MG, DG)                                                    \
    ADR REW(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

/* add (G = G + S)
 * set-flags: undefined (*x), yes (*z) */

#define addzx_ri(RG, IS)                                                    \
        addzz_ri(W(RG), W(IS))

#define addzx_mi(MG, DG, IS)                                                \
        addzz_mi(W(MG), W(DG), W(IS))

#define addzx_rr(RG, RS)                                                    \
        addzz_rr(W(RG), W(RS))

#define addzx_ld(RG, MS, DS)                                                \
        addzz_ld(W(RG), W(MS), W(DS))

#define addzx_st(RS, MG, DG)                                                \
        addzz_st(W(RS), W(MG), W(DG))

#define addzx_mr(MG, DG, RS)                                                \
        addzx_st(W(RS), W(MG), W(DG))


#define addzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x00,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define addzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x00,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define addzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define addzz_ld(RG, MS, DS)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define addzz_st(RS, MG, DG)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x01)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define addzz_mr(MG, DG, RS)                                                \
        addzz_st(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*x), yes (*z) */

#define subzx_ri(RG, IS)                                                    \
        subzz_ri(W(RG), W(IS))

#define subzx_mi(MG, DG, IS)                                                \
        subzz_mi(W(MG), W(DG), W(IS))

#define subzx_rr(RG, RS)                                                    \
        subzz_rr(W(RG), W(RS))

#define subzx_ld(RG, MS, DS)                                                \
        subzz_ld(W(RG), W(MS), W(DS))

#define subzx_st(RS, MG, DG)                                                \
        subzz_st(W(RS), W(MG), W(DG))

#define subzx_mr(MG, DG, RS)                                                \
        subzx_st(W(RS), W(MG), W(DG))


#define subzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define subzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IS))                         \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), CMD(IS))

#define subzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define subzz_ld(RG, MS, DS)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subzz_st(RS, MG, DG)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x29)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define subzz_mr(MG, DG, RS)                                                \
        subzz_st(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*x), yes (*z) */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlzx_rx(RG)                     /* reads Recx for shift count */   \
        shlzz_rx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlzx_rx(RG)                     /* reads Recx for shift count */   \
        VEW(RXB(RG), RXB(RG),    0x01, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shlzz_mx(W(MG), W(DG))

#define shlzx_ri(RG, IS)                                                    \
        shlzz_ri(W(RG), W(IS))

#define shlzx_mi(MG, DG, IS)                                                \
        shlzz_mi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzz_rr(W(RG), W(RS))

#define shlzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shlzz_ld(W(RG), W(MS), W(DS))

#define shlzx_st(RS, MG, DG)                                                \
        shlzz_st(W(RS), W(MG), W(DG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEW(RXB(RG), RXB(RG), REN(RS), 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shlzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
    ADR REW(0x01,    RXB(MS)) EMITB(0x8B)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        VEW(RXB(RG), RXB(RG),    0x0F, 0, 1, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shlzx_st(RS, MG, DG)                                                \
    ADR VEW(0x01,    RXB(MG), REN(RS), 0, 1, 2) EMITB(0xF7)                 \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)                                        \
    ADR REW(0x01,    RXB(MG)) EMITB(0x89)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shlzx_mr(MG, DG, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DG))


#define shlzz_rx(RG)                     /* reads Recx for shift count */   \
        REW(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \

#define shlzz_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REW(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shlzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shlzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x3F))

#define shlzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shlzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlzz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DS))                                        \
        shlzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlzz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shlzz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shlzz_mr(MG, DG, RS)                                                \
        shlzz_st(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*x), yes (*z) */

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrzx_rx(RG)                     /* reads Recx for shift count */   \
        shrzz_rx(W(RG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzx_rx(RG)                     /* reads Recx for shift count */   \
        VEW(RXB(RG), RXB(RG),    0x01, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        shrzz_mx(W(MG), W(DG))

#define shrzx_ri(RG, IS)                                                    \
        shrzz_ri(W(RG), W(IS))

#define shrzx_mi(MG, DG, IS)                                                \
        shrzz_mi(W(MG), W(DG), W(IS))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzz_rr(W(RG), W(RS))

#define shrzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        shrzz_ld(W(RG), W(MS), W(DS))

#define shrzx_st(RS, MG, DG)                                                \
        shrzz_st(W(RS), W(MG), W(DG))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEW(RXB(RG), RXB(RG), REN(RS), 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
    ADR REW(0x01,    RXB(MS)) EMITB(0x8B)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        VEW(RXB(RG), RXB(RG),    0x0F, 0, 3, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrzx_st(RS, MG, DG)                                                \
    ADR VEW(0x01,    RXB(MG), REN(RS), 0, 3, 2) EMITB(0xF7)                 \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)                                        \
    ADR REW(0x01,    RXB(MG)) EMITB(0x89)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzx_mr(MG, DG, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DG))


#define shrzz_rx(RG)                     /* reads Recx for shift count */   \
        REW(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \

#define shrzz_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REW(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x3F))

#define shrzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DS))                                        \
        shrzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define shrzz_mr(MG, DG, RS)                                                \
        shrzz_st(W(RS), W(MG), W(DG))


#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrzn_rx(RG)                     /* reads Recx for shift count */   \
        REW(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzn_rx(RG)                     /* reads Recx for shift count */   \
        VEW(RXB(RG), RXB(RG),    0x01, 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzn_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REW(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define shrzn_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define shrzn_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x3F))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DS))                                        \
        shrzn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzn_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzn_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        VEW(RXB(RG), RXB(RG), REN(RS), 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrzn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
    ADR REW(0x01,    RXB(MS)) EMITB(0x8B)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)                                        \
        VEW(RXB(RG), RXB(RG),    0x0F, 0, 2, 2) EMITB(0xF7)                 \
        MRM(REG(RG), MOD(RG), REG(RG))

#define shrzn_st(RS, MG, DG)                                                \
    ADR VEW(0x01,    RXB(MG), REN(RS), 0, 2, 2) EMITB(0xF7)                 \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)                                        \
    ADR REW(0x01,    RXB(MG)) EMITB(0x89)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define shrzn_mr(MG, DG, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 64 - S)
 * set-flags: undefined (*x), yes (*z) */

#define rorzx_rx(RG)                     /* reads Recx for shift count */   \
        rorzz_rx(W(RG))

#define rorzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        rorzz_mx(W(MG), W(DG))

#if RT_BASE_COMPAT_BMI < 2 /* 0 - generic, 1 - 3-op-VEX, 2 - BMI1+BMI2 */

#define rorzx_ri(RG, IS)                                                    \
        rorzz_ri(W(RG), W(IS))

#else /* RT_BASE_COMPAT_BMI >= 2 */

#define rorzx_ri(RG, IS)                                                    \
        VEW(RXB(RG), RXB(RG),    0x00, 0, 3, 3) EMITB(0xF0)                 \
        MRM(REG(RG), MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#endif /* RT_BASE_COMPAT_BMI >= 2 */

#define rorzx_mi(MG, DG, IS)                                                \
        rorzz_mi(W(MG), W(DG), W(IS))

#define rorzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        rorzz_rr(W(RG), W(RS))

#define rorzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        rorzz_ld(W(RG), W(MS), W(DS))

#define rorzx_st(RS, MG, DG)                                                \
        rorzz_st(W(RS), W(MG), W(DG))

#define rorzx_mr(MG, DG, RS)                                                \
        rorzx_st(W(RS), W(MG), W(DG))


#define rorzz_rx(RG)                     /* reads Recx for shift count */   \
        REW(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x01,    MOD(RG), REG(RG))                                      \

#define rorzz_mx(MG, DG)                 /* reads Recx for shift count */   \
    ADR REW(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMPTY)

#define rorzz_ri(RG, IS)                                                    \
        REW(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x01,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IS) & 0x3F))

#define rorzz_mi(MG, DG, IS)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DG), EMITB(VAL(IS) & 0x3F))

#define rorzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        rorzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define rorzz_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DS))                                        \
        rorzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define rorzz_st(RS, MG, DG)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        rorzz_mx(W(MG), W(DG))                                              \
        stack_ld(Recx)

#define rorzz_mr(MG, DG, RS)                                                \
        rorzz_st(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulzx_ri(RG, IS)                                                    \
        REW(RXB(RG), RXB(RG)) EMITB(0x69 | TYP(IS))                         \
        MRM(REG(RG), MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IS))

#define mulzx_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RS), REG(RS))

#define mulzx_ld(RG, MS, DS)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RS), REG(RS))                                      \

#define mulzx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RS), REG(RS))                                      \

#define mulzn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulzp_xr(RS)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xr(W(RS))       /* product must not exceed operands size */

#define mulzp_xm(MS, DS) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xm(W(MS), W(DS))/* product must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divzx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divzn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prezx_xx()          /* to be placed immediately prior divzx_x* */   \
        movzx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prezn_xx()          /* to be placed immediately prior divzn_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrzn_ri(Redx, IC(63))


#define divzx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REW(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divzx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divzn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define divzp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divzp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#define remzx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_xx()                                                          \
        divzx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_xx()                                                          \
        divzn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
                                     /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzx_xm(MS, DS)    /* to be placed immediately after divzx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
                                     /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzn_xm(MS, DS)    /* to be placed immediately after divzn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjzx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, zz_rx)                                               \
        CMJ(cc, lb)

#define arjzx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, zz_mx)                                        \
        CMJ(cc, lb)

#define arjzx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, zz_ri)                                        \
        CMJ(cc, lb)

#define arjzx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, zz_mi)                                 \
        CMJ(cc, lb)

#define arjzx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, zz_rr)                                        \
        CMJ(cc, lb)

#define arjzx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, zz_ld)                                 \
        CMJ(cc, lb)

#define arjzx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, zz_st)                                 \
        CMJ(cc, lb)

#define arjzx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjzx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjzx_rz(RS, cc, lb)                                                \
        cmjzx_ri(W(RS), IC(0), cc, lb)

#define cmjzx_mz(MS, DS, cc, lb)                                            \
        cmjzx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjzx_ri(RS, IT, cc, lb)                                            \
        cmpzx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjzx_mi(MS, DS, IT, cc, lb)                                        \
        cmpzx_mi(W(MS), W(DS), W(IT))                                       \
        CMJ(cc, lb)

#define cmjzx_rr(RS, RT, cc, lb)                                            \
        cmpzx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjzx_rm(RS, MT, DT, cc, lb)                                        \
        cmpzx_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjzx_mr(MS, DS, RT, cc, lb)                                        \
        cmpzx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpzx_ri(RS, IT)                                                    \
        REW(0,       RXB(RS)) EMITB(0x81 | TYP(IT))                         \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IT))

#define cmpzx_mi(MS, DS, IT)                                                \
    ADR REW(0,       RXB(MS)) EMITB(0x81 | TYP(IT))                         \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), CMD(IT))

#define cmpzx_rr(RS, RT)                                                    \
        REW(RXB(RS), RXB(RT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(RT), REG(RT))

#define cmpzx_rm(RS, MT, DT)                                                \
    ADR REW(RXB(RS), RXB(MT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DT), EMPTY)

#define cmpzx_mr(MS, DS, RT)                                                \
    ADR REW(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/* ver (Mebp/inf_VER = SIMD-version)
 * set-flags: no
 * For interpretation of SIMD build flags check compatibility layer in rtzero.h
 * 0th byte - 128-bit version, 1st byte - 256-bit version, | plus _R8/_RX slots
 * 2nd byte - 512-bit version, 3rd byte - 1K4-bit version, | in upper halves */

     /* verxx_xx() in 32-bit rtarch_***.h files, destroys Reax, ... , Redi */

/************************* address-sized instructions *************************/

/* adr (D = adr S)
 * set-flags: no */

     /* adrxx_ld(RD, MS, DS) is defined in 32-bit rtarch_***.h files */

/************************* pointer-sized instructions *************************/

/* label (D = Reax = adr lb)
 * set-flags: no */

     /* label_ld(lb) is defined in rtarch.h file, loads label to Reax */

     /* label_st(lb, MD, DD) is defined in rtarch.h file, destroys Reax */

/* jmp (if unconditional jump S/lb, else if cc flags then jump lb)
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, POWER:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, POWER:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

     /* jccxx_** is defined in 32-bit rtarch_***.h files */

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

     /* stack_** is defined in 32-bit rtarch_***.h files */

/******************************************************************************/
/**************************   extended double (x87)   *************************/
/******************************************************************************/

#define fpuzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDD)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuzs_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDD)                                   \
        MRM(0x03,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define fpuzn_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDF)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define fpuzn_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDF)                                   \
        MRM(0x07,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)

#define fpuzt_st(MD, DD) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MD)) EMITB(0xDD)                                   \
        MRM(0x01,    MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DD), EMPTY)


#define addzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x00,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define subzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define sbrzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)


#define mulzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x01,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define divzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

#define dvrzs_ld(MS, DS) /* not portable, do not use outside */             \
    ADR REX(0,       RXB(MS)) EMITB(0xDC)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DS), EMPTY)

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_X64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

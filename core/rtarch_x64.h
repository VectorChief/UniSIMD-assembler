/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
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
 * IM - immediate value (smallest size IC is used for shifts)
 * DP - displacement value (of given size DP, DF, DG, DH, DV)
 *
 * Mixing of 64/32-bit fields in backend structures may lead to misalignment
 * of 64-bit fields to 4-byte boundary, which is not supported on some targets.
 * Place fields carefully to ensure proper alignment for all data types.
 * Note that within cmdx*_** subset most of the instructions follow in-heap
 * address size (RT_ADDRESS or A) and only label_ld/st, jmpxx_xr/xm follow
 * pointer size (RT_POINTER or P) as code/data/stack segments are fixed.
 * In 64/32-bit (ptr/adr) hybrid mode there is no way to move 64-bit registers,
 * thus label_ld has very limited use as jmpxx_xr(Reax) is the only matching op.
 * Stack ops always work with full registers regardless of the mode chosen.
 *
 * 32-bit and 64-bit BASE subsets are not easily compatible on all targets,
 * thus any register modified with 32-bit op cannot be used in 64-bit subset.
 * Alternatively, data flow must not exceed 31-bit range for 32-bit operations
 * to produce consistent results usable in 64-bit subset across all targets.
 * Only a64 and x64 have a complete 32-bit support in 64-bit mode both zeroing
 * the upper half of the result, while m64 sign-extending all 32-bit operations
 * and p64 overflowing 32-bit arithmetic into the upper half. Similar reasons
 * of inconsistency prohibit use of IW immediate type within 64-bit subset,
 * where a64 and p64 zero-extend, while x64 and m64 sign-extend 32-bit value.
 *
 * Note that offset correction for endianness E is only applicable for addresses
 * within pointer fields, when (in-heap) address and pointer sizes don't match.
 * Working with 32-bit data in 64-bit fields in any other circumstances must be
 * done consistently within a subset of one size (32-bit, 64-bit or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 *
 * Setting-flags instructions' naming scheme may change again in the future for
 * better orthogonality with operands size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on certain targets use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainder operations can only be done natively on MIPS.
 * Consider using special fixed-register forms for maximum performance.
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
/**********************************   X64   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movzx_ri(RD, IM)                                                    \
        REW(0,       RXB(RD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RD), REG(RD))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movzx_mi(MD, DP, IM)                                                \
    ADR REW(0,       RXB(MD)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(MD), REG(MD))   /* truncate IC with TYP below */   \
        AUX(SIB(MD), CMD(DP), EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movzx_rr(RD, RS)                                                    \
        REW(RXB(RD), RXB(RS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(RS), REG(RS))

#define movzx_ld(RD, MS, DP)                                                \
    ADR REW(RXB(RD), RXB(MS)) EMITB(0x8B)                                   \
        MRM(REG(RD), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)

#define movzx_st(RS, MD, DP)                                                \
    ADR REW(RXB(RS), RXB(MD)) EMITB(0x89)                                   \
        MRM(REG(RS), MOD(MD), REG(MD))                                      \
        AUX(SIB(MD), CMD(DP), EMPTY)

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andzx_ri(RG, IM)                                                    \
        andzz_ri(W(RG), W(IM))

#define andzx_mi(MG, DP, IM)                                                \
        andzz_mi(W(MG), W(DP), W(IM))

#define andzx_rr(RG, RS)                                                    \
        andzz_rr(W(RG), W(RS))

#define andzx_ld(RG, MS, DP)                                                \
        andzz_ld(W(RG), W(MS), W(DP))

#define andzx_st(RS, MG, DP)                                                \
        andzz_st(W(RS), W(MG), W(DP))


#define andzz_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andzz_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), CMD(IM))

#define andzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define andzz_ld(RG, MS, DP)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)

#define andzz_st(RS, MG, DP)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x21)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annzx_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        andzx_ri(W(RG), W(IM))

#define annzx_mi(MG, DP, IM)                                                \
        notzx_rx(W(MG), W(DP))                                              \
        andzx_mi(W(MG), W(DP), W(IM))

#define annzx_rr(RG, RS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzx_rr(W(RG), W(RS))

#define annzx_ld(RG, MS, DP)                                                \
        notzx_rx(W(RG))                                                     \
        andzx_ld(W(RG), W(MS), W(DP))

#define annzx_st(RS, MG, DP)                                                \
        notzx_rx(W(MG), W(DP))                                              \
        andzx_st(W(RS), W(MG), W(DP))

#define annzx_mr(MG, DP, RS)                                                \
        annzx_st(W(RS), W(MG), W(DP))


#define annzz_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        andzz_ri(W(RG), W(IM))

#define annzz_mi(MG, DP, IM)                                                \
        notzx_rx(W(MG), W(DP))                                              \
        andzz_mi(W(MG), W(DP), W(IM))

#define annzz_rr(RG, RS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzz_rr(W(RG), W(RS))

#define annzz_ld(RG, MS, DP)                                                \
        notzx_rx(W(RG))                                                     \
        andzz_ld(W(RG), W(MS), W(DP))

#define annzz_st(RS, MG, DP)                                                \
        notzx_rx(W(MG), W(DP))                                              \
        andzz_st(W(RS), W(MG), W(DP))

#define annzz_mr(MG, DP, RS)                                                \
        annzz_st(W(RS), W(MG), W(DP))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrzx_ri(RG, IM)                                                    \
        orrzz_ri(W(RG), W(IM))

#define orrzx_mi(MG, DP, IM)                                                \
        orrzz_mi(W(MG), W(DP), W(IM))

#define orrzx_rr(RG, RS)                                                    \
        orrzz_rr(W(RG), W(RS))

#define orrzx_ld(RG, MS, DP)                                                \
        orrzz_ld(W(RG), W(MS), W(DP))

#define orrzx_st(RS, MG, DP)                                                \
        orrzz_st(W(RS), W(MG), W(DP))


#define orrzz_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrzz_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), CMD(IM))

#define orrzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define orrzz_ld(RG, MS, DP)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)

#define orrzz_st(RS, MG, DP)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x09)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornzx_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzx_ri(W(RG), W(IM))

#define ornzx_mi(MG, DP, IM)                                                \
        notzx_mx(W(MG), W(DP))                                              \
        orrzx_mi(W(MG), W(DP), W(IM))

#define ornzx_rr(RG, RS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzx_rr(W(RG), W(RS))

#define ornzx_ld(RG, MS, DP)                                                \
        notzx_rx(W(RG))                                                     \
        orrzx_ld(W(RG), W(MS), W(DP))

#define ornzx_st(RS, MG, DP)                                                \
        notzx_mx(W(MG), W(DP))                                              \
        orrzx_st(W(RS), W(MG), W(DP))

#define ornzx_mr(MG, DP, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DP))


#define ornzz_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzz_ri(W(RG), W(IM))

#define ornzz_mi(MG, DP, IM)                                                \
        notzx_mx(W(MG), W(DP))                                              \
        orrzz_mi(W(MG), W(DP), W(IM))

#define ornzz_rr(RG, RS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzz_rr(W(RG), W(RS))

#define ornzz_ld(RG, MS, DP)                                                \
        notzx_rx(W(RG))                                                     \
        orrzz_ld(W(RG), W(MS), W(DP))

#define ornzz_st(RS, MG, DP)                                                \
        notzx_mx(W(MG), W(DP))                                              \
        orrzz_st(W(RS), W(MG), W(DP))

#define ornzz_mr(MG, DP, RS)                                                \
        ornzz_st(W(RS), W(MG), W(DP))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorzx_ri(RG, IM)                                                    \
        xorzz_ri(W(RG), W(IM))

#define xorzx_mi(MG, DP, IM)                                                \
        xorzz_mi(W(MG), W(DP), W(IM))

#define xorzx_rr(RG, RS)                                                    \
        xorzz_rr(W(RG), W(RS))

#define xorzx_ld(RG, MS, DP)                                                \
        xorzz_ld(W(RG), W(MS), W(DP))

#define xorzx_st(RS, MG, DP)                                                \
        xorzz_st(W(RS), W(MG), W(DP))


#define xorzz_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define xorzz_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), CMD(IM))

#define xorzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define xorzz_ld(RG, MS, DP)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)

#define xorzz_st(RS, MG, DP)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x31)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

/* not
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        REW(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RG), REG(RG))

#define notzx_mx(MG, DP)                                                    \
    ADR REW(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negzx_rx(RG)                                                        \
        negzz_rx(W(RG))

#define negzx_mx(MG, DP)                                                    \
        negzz_mx(W(MG), W(DP))


#define negzz_rx(RG)                                                        \
        REW(0,       RXB(RG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RG), REG(RG))

#define negzz_mx(MG, DP)                                                    \
    ADR REW(0,       RXB(MG)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addzx_ri(RG, IM)                                                    \
        addzz_ri(W(RG), W(IM))

#define addzx_mi(MG, DP, IM)                                                \
        addzz_mi(W(MG), W(DP), W(IM))

#define addzx_rr(RG, RS)                                                    \
        addzz_rr(W(RG), W(RS))

#define addzx_ld(RG, MS, DP)                                                \
        addzz_ld(W(RG), W(MS), W(DP))

#define addzx_st(RS, MG, DP)                                                \
        addzz_st(W(RS), W(MG), W(DP))


#define addzz_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addzz_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), CMD(IM))

#define addzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define addzz_ld(RG, MS, DP)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)

#define addzz_st(RS, MG, DP)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x01)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subzx_ri(RG, IM)                                                    \
        subzz_ri(W(RG), W(IM))

#define subzx_mi(MG, DP, IM)                                                \
        subzz_mi(W(MG), W(DP), W(IM))

#define subzx_rr(RG, RS)                                                    \
        subzz_rr(W(RG), W(RS))

#define subzx_ld(RG, MS, DP)                                                \
        subzz_ld(W(RG), W(MS), W(DP))

#define subzx_st(RS, MG, DP)                                                \
        subzz_st(W(RS), W(MG), W(DP))

#define subzx_mr(MG, DP, RS)                                                \
        subzx_st(W(RS), W(MG), W(DP))


#define subzz_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subzz_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), CMD(IM))

#define subzz_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RS), REG(RS))

#define subzz_ld(RG, MS, DP)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)

#define subzz_st(RS, MG, DP)                                                \
    ADR REW(RXB(RS), RXB(MG)) EMITB(0x29)                                   \
        MRM(REG(RS), MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

#define subzz_mr(MG, DP, RS)                                                \
        subzz_st(W(RS), W(MG), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlzx_rx(RG)                     /* reads Recx for shift value */   \
        shlzz_rx(W(RG))

#define shlzx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shlzz_mx(W(MG), W(DP))

#define shlzx_ri(RG, IM)                                                    \
        shlzz_ri(W(RG), W(IM))

#define shlzx_mi(MG, DP, IM)                                                \
        shlzz_mi(W(MG), W(DP), W(IM))

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shlzz_rr(W(RG), W(RS))

#define shlzx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shlzz_ld(W(RG), W(MS), W(DP))

#define shlzx_st(RS, MG, DP)                                                \
        shlzz_st(W(RS), W(MG), W(DP))

#define shlzx_mr(MG, DP, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DP))


#define shlzz_rx(RG)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \

#define shlzz_mx(MG, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

#define shlzz_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlzz_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shlzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shlzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlzz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DP))                                        \
        shlzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlzz_st(RS, MG, DP)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shlzz_mx(W(MG), W(DP))                                              \
        stack_ld(Recx)

#define shlzz_mr(MG, DP, RS)                                                \
        shlzz_st(W(RS), W(MG), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrzx_rx(RG)                     /* reads Recx for shift value */   \
        shrzz_rx(W(RG))

#define shrzx_mx(MG, DP)                 /* reads Recx for shift value */   \
        shrzz_mx(W(MG), W(DP))

#define shrzx_ri(RG, IM)                                                    \
        shrzz_ri(W(RG), W(IM))

#define shrzx_mi(MG, DP, IM)                                                \
        shrzz_mi(W(MG), W(DP), W(IM))

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        shrzz_rr(W(RG), W(RS))

#define shrzx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        shrzz_ld(W(RG), W(MS), W(DP))

#define shrzx_st(RS, MG, DP)                                                \
        shrzz_st(W(RS), W(MG), W(DP))

#define shrzx_mr(MG, DP, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DP))


#define shrzz_rx(RG)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \

#define shrzz_mx(MG, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

#define shrzz_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrzz_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shrzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DP))                                        \
        shrzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzz_st(RS, MG, DP)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzz_mx(W(MG), W(DP))                                              \
        stack_ld(Recx)

#define shrzz_mr(MG, DP, RS)                                                \
        shrzz_st(W(RS), W(MG), W(DP))


#define shrzn_rx(RG)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \

#define shrzn_mx(MG, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(MG)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMPTY)

#define shrzn_ri(RG, IM)                                                    \
        REW(0,       RXB(RG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrzn_mi(MG, DP, IM)                                                \
    ADR REW(0,       RXB(MG)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(MG), REG(MG))                                      \
        AUX(SIB(MG), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzn_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(MS), W(DP))                                        \
        shrzn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzn_st(RS, MG, DP)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RS))                                               \
        shrzn_mx(W(MG), W(DP))                                              \
        stack_ld(Recx)

#define shrzn_mr(MG, DP, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DP))

/* mul
 * set-flags: undefined */

#define mulzx_ri(RG, IM)                                                    \
        REW(RXB(RG), RXB(RG)) EMITB(0x69 | TYP(IM))                         \
        MRM(REG(RG), MOD(RG), REG(RG))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulzx_rr(RG, RS)                                                    \
        REW(RXB(RG), RXB(RS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RS), REG(RS))

#define mulzx_ld(RG, MS, DP)                                                \
    ADR REW(RXB(RG), RXB(MS)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RS), REG(RS))                                      \

#define mulzx_xm(MS, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RS), REG(RS))                                      \

#define mulzn_xm(MS, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)


#define mulzp_xr(RS)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xr(W(RS))       /* product must not exceed operands size */

#define mulzp_xm(MS, DP) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xm(W(MS), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divzx_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzx_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xm(W(MS), W(DP))                                              \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divzn_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzn_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xm(W(MS), W(DP))                                              \
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

#define divzx_xm(MS, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(RS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divzn_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(MS)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)


#define divzp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divzp_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remzx_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzx_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xm(W(MS), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzn_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzn_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xm(W(MS), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
                                     /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzx_xm(MS, DP)    /* to be placed immediately after divzx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
                                     /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzn_xm(MS, DP)    /* to be placed immediately after divzn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define arjzx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, zz_rx)                                               \
        CMJ(cc, lb)

#define arjzx_mx(MG, DP, op, cc, lb)                                        \
        AR2(W(MG), W(DP), op, zz_mx)                                        \
        CMJ(cc, lb)

#define arjzx_ri(RG, IM, op, cc, lb)                                        \
        AR2(W(RG), W(IM), op, zz_ri)                                        \
        CMJ(cc, lb)

#define arjzx_mi(MG, DP, IM, op, cc, lb)                                    \
        AR3(W(MG), W(DP), W(IM), op, zz_mi)                                 \
        CMJ(cc, lb)

#define arjzx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, zz_rr)                                        \
        CMJ(cc, lb)

#define arjzx_ld(RG, MS, DP, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DP), op, zz_ld)                                 \
        CMJ(cc, lb)

#define arjzx_st(RS, MG, DP, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DP), op, zz_st)                                 \
        CMJ(cc, lb)

#define arjzx_mr(MG, DP, RS, op, cc, lb)                                    \
        arjzx_st(W(RS), W(MG), W(DP), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjzx_rz(RS, cc, lb)                                                \
        cmjzx_ri(W(RS), IC(0), cc, lb)

#define cmjzx_mz(MS, DP, cc, lb)                                            \
        cmjzx_mi(W(MS), W(DP), IC(0), cc, lb)

#define cmjzx_ri(RS, IM, cc, lb)                                            \
        cmpzx_ri(W(RS), W(IM))                                              \
        CMJ(cc, lb)

#define cmjzx_mi(MS, DP, IM, cc, lb)                                        \
        cmpzx_mi(W(MS), W(DP), W(IM))                                       \
        CMJ(cc, lb)

#define cmjzx_rr(RS, RT, cc, lb)                                            \
        cmpzx_rr(W(RS), W(RT))                                              \
        CMJ(cc, lb)

#define cmjzx_rm(RS, MT, DP, cc, lb)                                        \
        cmpzx_rm(W(RS), W(MT), W(DP))                                       \
        CMJ(cc, lb)

#define cmjzx_mr(MS, DP, RT, cc, lb)                                        \
        cmpzx_mr(W(MS), W(DP), W(RT))                                       \
        CMJ(cc, lb)

/* cmp
 * set-flags: yes */

#define cmpzx_ri(RS, IM)                                                    \
        REW(0,       RXB(RS)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(RS), REG(RS))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpzx_mi(MS, DP, IM)                                                \
    ADR REW(0,       RXB(MS)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), CMD(IM))

#define cmpzx_rr(RS, RT)                                                    \
        REW(RXB(RS), RXB(RT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(RT), REG(RT))

#define cmpzx_rm(RS, MT, DP)                                                \
    ADR REW(RXB(RS), RXB(MT)) EMITB(0x3B)                                   \
        MRM(REG(RS), MOD(MT), REG(MT))                                      \
        AUX(SIB(MT), CMD(DP), EMPTY)

#define cmpzx_mr(MS, DP, RT)                                                \
    ADR REW(RXB(RT), RXB(MS)) EMITB(0x39)                                   \
        MRM(REG(RT), MOD(MS), REG(MS))                                      \
        AUX(SIB(MS), CMD(DP), EMPTY)

#endif /* RT_RTARCH_X64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

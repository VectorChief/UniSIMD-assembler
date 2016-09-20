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

#define movzx_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movzx_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(SIB(RM), CMD(DP), EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movzx_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x8B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movzx_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x8B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movzx_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x89)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andzx_ri(RM, IM)                                                    \
        andzz_ri(W(RM), W(IM))

#define andzx_mi(RM, DP, IM)                                                \
        andzz_mi(W(RM), W(DP), W(IM))

#define andzx_rr(RG, RM)                                                    \
        andzz_rr(W(RG), W(RM))

#define andzx_ld(RG, RM, DP)                                                \
        andzz_ld(W(RG), W(RM), W(DP))

#define andzx_st(RG, RM, DP)                                                \
        andzz_st(W(RG), W(RM), W(DP))


#define andzz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andzz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define andzz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andzz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andzz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x21)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* ann
 * set-flags: undefined (*x), yes (*z) */

#define annzx_ri(RM, IM)                                                    \
        notzx_rx(W(RM))                                                     \
        andzx_ri(W(RM), W(IM))

#define annzx_mi(RM, DP, IM)                                                \
        notzx_rx(W(RM), W(DP))                                              \
        andzx_mi(W(RM), W(DP), W(IM))

#define annzx_rr(RG, RM)                                                    \
        notzx_rx(W(RG))                                                     \
        andzx_rr(W(RG), W(RM))

#define annzx_ld(RG, RM, DP)                                                \
        notzx_rx(W(RG))                                                     \
        andzx_ld(W(RG), W(RM), W(DP))

#define annzx_st(RG, RM, DP)                                                \
        notzx_rx(W(RM), W(DP))                                              \
        andzx_st(W(RG), W(RM), W(DP))

#define annzx_mr(RM, DP, RG)                                                \
        annzx_st(W(RG), W(RM), W(DP))


#define annzz_ri(RM, IM)                                                    \
        notzx_rx(W(RM))                                                     \
        andzz_ri(W(RM), W(IM))

#define annzz_mi(RM, DP, IM)                                                \
        notzx_rx(W(RM), W(DP))                                              \
        andzz_mi(W(RM), W(DP), W(IM))

#define annzz_rr(RG, RM)                                                    \
        notzx_rx(W(RG))                                                     \
        andzz_rr(W(RG), W(RM))

#define annzz_ld(RG, RM, DP)                                                \
        notzx_rx(W(RG))                                                     \
        andzz_ld(W(RG), W(RM), W(DP))

#define annzz_st(RG, RM, DP)                                                \
        notzx_rx(W(RM), W(DP))                                              \
        andzz_st(W(RG), W(RM), W(DP))

#define annzz_mr(RM, DP, RG)                                                \
        annzz_st(W(RG), W(RM), W(DP))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrzx_ri(RM, IM)                                                    \
        orrzz_ri(W(RM), W(IM))

#define orrzx_mi(RM, DP, IM)                                                \
        orrzz_mi(W(RM), W(DP), W(IM))

#define orrzx_rr(RG, RM)                                                    \
        orrzz_rr(W(RG), W(RM))

#define orrzx_ld(RG, RM, DP)                                                \
        orrzz_ld(W(RG), W(RM), W(DP))

#define orrzx_st(RG, RM, DP)                                                \
        orrzz_st(W(RG), W(RM), W(DP))


#define orrzz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrzz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrzz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrzz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define orrzz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x09)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orn
 * set-flags: undefined (*x), yes (*z) */

#define ornzx_ri(RM, IM)                                                    \
        notzx_rx(W(RM))                                                     \
        orrzx_ri(W(RM), W(IM))

#define ornzx_mi(RM, DP, IM)                                                \
        notzx_mx(W(RM), W(DP))                                              \
        orrzx_mi(W(RM), W(DP), W(IM))

#define ornzx_rr(RG, RM)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzx_rr(W(RG), W(RM))

#define ornzx_ld(RG, RM, DP)                                                \
        notzx_rx(W(RG))                                                     \
        orrzx_ld(W(RG), W(RM), W(DP))

#define ornzx_st(RG, RM, DP)                                                \
        notzx_mx(W(RM), W(DP))                                              \
        orrzx_st(W(RG), W(RM), W(DP))

#define ornzx_mr(RM, DP, RG)                                                \
        ornzx_st(W(RG), W(RM), W(DP))


#define ornzz_ri(RM, IM)                                                    \
        notzx_rx(W(RM))                                                     \
        orrzz_ri(W(RM), W(IM))

#define ornzz_mi(RM, DP, IM)                                                \
        notzx_mx(W(RM), W(DP))                                              \
        orrzz_mi(W(RM), W(DP), W(IM))

#define ornzz_rr(RG, RM)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzz_rr(W(RG), W(RM))

#define ornzz_ld(RG, RM, DP)                                                \
        notzx_rx(W(RG))                                                     \
        orrzz_ld(W(RG), W(RM), W(DP))

#define ornzz_st(RG, RM, DP)                                                \
        notzx_mx(W(RM), W(DP))                                              \
        orrzz_st(W(RG), W(RM), W(DP))

#define ornzz_mr(RM, DP, RG)                                                \
        ornzz_st(W(RG), W(RM), W(DP))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorzx_ri(RM, IM)                                                    \
        xorzz_ri(W(RM), W(IM))

#define xorzx_mi(RM, DP, IM)                                                \
        xorzz_mi(W(RM), W(DP), W(IM))

#define xorzx_rr(RG, RM)                                                    \
        xorzz_rr(W(RG), W(RM))

#define xorzx_ld(RG, RM, DP)                                                \
        xorzz_ld(W(RG), W(RM), W(DP))

#define xorzx_st(RG, RM, DP)                                                \
        xorzz_st(W(RG), W(RM), W(DP))


#define xorzz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define xorzz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define xorzz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorzz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define xorzz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x31)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not
 * set-flags: no */

#define notzx_rx(RM)                                                        \
        REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RM), REG(RM))

#define notzx_mx(RM, DP)                                                    \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negzx_rx(RM)                                                        \
        negzz_rx(W(RM))

#define negzx_mx(RM, DP)                                                    \
        negzz_mx(W(RM), W(DP))


#define negzz_rx(RM)                                                        \
        REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))

#define negzz_mx(RM, DP)                                                    \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addzx_ri(RM, IM)                                                    \
        addzz_ri(W(RM), W(IM))

#define addzx_mi(RM, DP, IM)                                                \
        addzz_mi(W(RM), W(DP), W(IM))

#define addzx_rr(RG, RM)                                                    \
        addzz_rr(W(RG), W(RM))

#define addzx_ld(RG, RM, DP)                                                \
        addzz_ld(W(RG), W(RM), W(DP))

#define addzx_st(RG, RM, DP)                                                \
        addzz_st(W(RG), W(RM), W(DP))


#define addzz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addzz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define addzz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addzz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addzz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x01)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subzx_ri(RM, IM)                                                    \
        subzz_ri(W(RM), W(IM))

#define subzx_mi(RM, DP, IM)                                                \
        subzz_mi(W(RM), W(DP), W(IM))

#define subzx_rr(RG, RM)                                                    \
        subzz_rr(W(RG), W(RM))

#define subzx_ld(RG, RM, DP)                                                \
        subzz_ld(W(RG), W(RM), W(DP))

#define subzx_st(RG, RM, DP)                                                \
        subzz_st(W(RG), W(RM), W(DP))

#define subzx_mr(RM, DP, RG)                                                \
        subzx_st(W(RG), W(RM), W(DP))


#define subzz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subzz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define subzz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subzz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subzz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x29)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subzz_mr(RM, DP, RG)                                                \
        subzz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlzx_rx(RM)                     /* reads Recx for shift value */   \
        shlzz_rx(W(RM))

#define shlzx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlzz_mx(W(RM), W(DP))

#define shlzx_ri(RM, IM)                                                    \
        shlzz_ri(W(RM), W(IM))

#define shlzx_mi(RM, DP, IM)                                                \
        shlzz_mi(W(RM), W(DP), W(IM))

#define shlzx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlzz_rr(W(RG), W(RM))

#define shlzx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlzz_ld(W(RG), W(RM), W(DP))

#define shlzx_st(RG, RM, DP)                                                \
        shlzz_st(W(RG), W(RM), W(DP))

#define shlzx_mr(RM, DP, RG)                                                \
        shlzx_st(W(RG), W(RM), W(DP))


#define shlzz_rx(RM)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define shlzz_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shlzz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlzz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shlzz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RM))                                               \
        shlzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlzz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(RM), W(DP))                                        \
        shlzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlzz_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RG))                                               \
        shlzz_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shlzz_mr(RM, DP, RG)                                                \
        shlzz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrzx_rx(RM)                     /* reads Recx for shift value */   \
        shrzz_rx(W(RM))

#define shrzx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrzz_mx(W(RM), W(DP))

#define shrzx_ri(RM, IM)                                                    \
        shrzz_ri(W(RM), W(IM))

#define shrzx_mi(RM, DP, IM)                                                \
        shrzz_mi(W(RM), W(DP), W(IM))

#define shrzx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrzz_rr(W(RG), W(RM))

#define shrzx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrzz_ld(W(RG), W(RM), W(DP))

#define shrzx_st(RG, RM, DP)                                                \
        shrzz_st(W(RG), W(RM), W(DP))

#define shrzx_mr(RM, DP, RG)                                                \
        shrzx_st(W(RG), W(RM), W(DP))


#define shrzz_rx(RM)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define shrzz_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrzz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrzz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shrzz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RM))                                               \
        shrzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(RM), W(DP))                                        \
        shrzz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzz_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RG))                                               \
        shrzz_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shrzz_mr(RM, DP, RG)                                                \
        shrzz_st(W(RG), W(RM), W(DP))


#define shrzn_rx(RM)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \

#define shrzn_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrzn_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrzn_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shrzn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RM))                                               \
        shrzn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movzx_ld(Recx, W(RM), W(DP))                                        \
        shrzn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrzn_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movzx_rr(Recx, W(RG))                                               \
        shrzn_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shrzn_mr(RM, DP, RG)                                                \
        shrzn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulzx_ri(RM, IM)                                                    \
        REW(RXB(RM), RXB(RM)) EMITB(0x69 | TYP(IM))                         \
        MRM(REG(RM), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulzx_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulzx_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulzx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define mulzx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulzn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define mulzn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulzp_xr(RM)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xr(W(RM))       /* product must not exceed operands size */

#define mulzp_xm(RM, DP) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzn_xm(W(RM), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divzx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RM))                                               \
        prezx_rr()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RM), Reax)                                               \
        stack_ld(Reax)

#define divzx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xr(W(RM))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xm(W(RM), W(DP))                                              \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divzn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RM))                                               \
        prezn_rr()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RM), Reax)                                               \
        stack_ld(Reax)

#define divzn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xr(W(RM))                                                     \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divzn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xm(W(RM), W(DP))                                              \
        stack_ld(Redx)                                                      \
        movzx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prezx_xx()          /* to be placed immediately prior divzx_x* */   \
        movzx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prezn_xx()          /* to be placed immediately prior divzn_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrzn_ri(Redx, IC(63))


#define divzx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divzx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define divzn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divzn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define divzp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divzp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remzx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RM))                                               \
        prezx_rr()                                                          \
        divzx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RM), Redx)                                               \
        stack_ld(Redx)

#define remzx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xr(W(RM))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezx_rr()                                                          \
        divzx_xm(W(RM), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movzx_rr(Reax, W(RM))                                               \
        prezn_rr()                                                          \
        divzn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RM), Redx)                                               \
        stack_ld(Redx)

#define remzn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xr(W(RM))                                                     \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remzn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movzx_rr(Reax, W(RG))                                               \
        prezn_rr()                                                          \
        divzn_xm(W(RM), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movzx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
                                     /* to prepare for rem calculation */

#define remzx_xr(RM)        /* to be placed immediately after divzx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzx_xm(RM, DP)    /* to be placed immediately after divzx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
                                     /* to prepare for rem calculation */

#define remzn_xr(RM)        /* to be placed immediately after divzn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remzn_xm(RM, DP)    /* to be placed immediately after divzn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define arjzx_rx(RM, op, cc, lb)                                            \
        AR1(W(RM), op, zz_rx)                                               \
        CMJ(cc, lb)

#define arjzx_mx(RM, DP, op, cc, lb)                                        \
        AR2(W(RM), W(DP), op, zz_mx)                                        \
        CMJ(cc, lb)

#define arjzx_ri(RM, IM, op, cc, lb)                                        \
        AR2(W(RM), W(IM), op, zz_ri)                                        \
        CMJ(cc, lb)

#define arjzx_mi(RM, DP, IM, op, cc, lb)                                    \
        AR3(W(RM), W(DP), W(IM), op, zz_mi)                                 \
        CMJ(cc, lb)

#define arjzx_rr(RG, RM, op, cc, lb)                                        \
        AR2(W(RG), W(RM), op, zz_rr)                                        \
        CMJ(cc, lb)

#define arjzx_ld(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, zz_ld)                                 \
        CMJ(cc, lb)

#define arjzx_st(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, zz_st)                                 \
        CMJ(cc, lb)

#define arjzx_mr(RM, DP, RG, op, cc, lb)                                    \
        arjzx_st(W(RG), W(RM), W(DP), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjzx_rz(RM, cc, lb)                                                \
        cmjzx_ri(W(RM), IC(0), cc, lb)

#define cmjzx_mz(RM, DP, cc, lb)                                            \
        cmjzx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjzx_ri(RM, IM, cc, lb)                                            \
        cmpzx_ri(W(RM), W(IM))                                              \
        CMJ(cc, lb)

#define cmjzx_mi(RM, DP, IM, cc, lb)                                        \
        cmpzx_mi(W(RM), W(DP), W(IM))                                       \
        CMJ(cc, lb)

#define cmjzx_rr(RG, RM, cc, lb)                                            \
        cmpzx_rr(W(RG), W(RM))                                              \
        CMJ(cc, lb)

#define cmjzx_rm(RG, RM, DP, cc, lb)                                        \
        cmpzx_rm(W(RG), W(RM), W(DP))                                       \
        CMJ(cc, lb)

#define cmjzx_mr(RM, DP, RG, cc, lb)                                        \
        cmpzx_mr(W(RM), W(DP), W(RG))                                       \
        CMJ(cc, lb)

/* cmp
 * set-flags: yes */

#define cmpzx_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpzx_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpzx_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x3B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cmpzx_rm(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x3B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpzx_mr(RM, DP, RG)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x39)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_RTARCH_X64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

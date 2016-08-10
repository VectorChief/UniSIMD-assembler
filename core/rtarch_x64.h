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
 * rtarch_x64.h: Implementation of x86_64 BASE instructions.
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
 * The cmdw*_** and cmdx*_** subsets are not easily compatible on all targets,
 * thus any register modified by cmdw*_** cannot be used in cmdx*_** subset.
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
 * done consistently within a subset of one size (cmdw*_**, cmdx*_** or C/C++).
 * Alternatively, data written natively in C/C++ can be worked on from within
 * a given (one) subset if appropriate offset correction is used from rtarch.h.
 * Mixing of cmdw*_** and cmdx*_** without C/C++ is supported via F definition,
 * but requires two offsets for each field, with F for (w*) and plain for (x*).
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

#define movxx_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(EMPTY,   EMPTY,   EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movxx_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC7)                                   \
        MRM(0x00,    MOD(RM), REG(RM))   /* truncate IC with TYP below */   \
        AUX(SIB(RM), CMD(DP), EMITW(VAL(IM) & ((TYP(IM) << 6) - 1)))

#define movxx_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x8B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define movxx_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x8B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define movxx_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x89)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andxx_ri(RM, IM)                                                    \
        andxz_ri(W(RM), W(IM))

#define andxx_mi(RM, DP, IM)                                                \
        andxz_mi(W(RM), W(DP), W(IM))

#define andxx_rr(RG, RM)                                                    \
        andxz_rr(W(RG), W(RM))

#define andxx_ld(RG, RM, DP)                                                \
        andxz_ld(W(RG), W(RM), W(DP))

#define andxx_st(RG, RM, DP)                                                \
        andxz_st(W(RG), W(RM), W(DP))


#define andxz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define andxz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define andxz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define andxz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x23)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define andxz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x21)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrxx_ri(RM, IM)                                                    \
        orrxz_ri(W(RM), W(IM))

#define orrxx_mi(RM, DP, IM)                                                \
        orrxz_mi(W(RM), W(DP), W(IM))

#define orrxx_rr(RG, RM)                                                    \
        orrxz_rr(W(RG), W(RM))

#define orrxx_ld(RG, RM, DP)                                                \
        orrxz_ld(W(RG), W(RM), W(DP))

#define orrxx_st(RG, RM, DP)                                                \
        orrxz_st(W(RG), W(RM), W(DP))


#define orrxz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define orrxz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x01,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define orrxz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define orrxz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x0B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define orrxz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x09)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorxx_ri(RM, IM)                                                    \
        xorxz_ri(W(RM), W(IM))

#define xorxx_mi(RM, DP, IM)                                                \
        xorxz_mi(W(RM), W(DP), W(IM))

#define xorxx_rr(RG, RM)                                                    \
        xorxz_rr(W(RG), W(RM))

#define xorxx_ld(RG, RM, DP)                                                \
        xorxz_ld(W(RG), W(RM), W(DP))

#define xorxx_st(RG, RM, DP)                                                \
        xorxz_st(W(RG), W(RM), W(DP))


#define xorxz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define xorxz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define xorxz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define xorxz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x33)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define xorxz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x31)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* not
 * set-flags: no */

#define notxx_rx(RM)                                                        \
        REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RM), REG(RM))

#define notxx_mx(RM, DP)                                                    \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x02,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negxx_rx(RM)                                                        \
        negxz_rx(W(RM))

#define negxx_mx(RM, DP)                                                    \
        negxz_mx(W(RM), W(DP))


#define negxz_rx(RM)                                                        \
        REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))

#define negxz_mx(RM, DP)                                                    \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x03,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addxx_ri(RM, IM)                                                    \
        addxz_ri(W(RM), W(IM))

#define addxx_mi(RM, DP, IM)                                                \
        addxz_mi(W(RM), W(DP), W(IM))

#define addxx_rr(RG, RM)                                                    \
        addxz_rr(W(RG), W(RM))

#define addxx_ld(RG, RM, DP)                                                \
        addxz_ld(W(RG), W(RM), W(DP))

#define addxx_st(RG, RM, DP)                                                \
        addxz_st(W(RG), W(RM), W(DP))


#define addxz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define addxz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x00,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define addxz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define addxz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x03)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define addxz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x01)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subxx_ri(RM, IM)                                                    \
        subxz_ri(W(RM), W(IM))

#define subxx_mi(RM, DP, IM)                                                \
        subxz_mi(W(RM), W(DP), W(IM))

#define subxx_rr(RG, RM)                                                    \
        subxz_rr(W(RG), W(RM))

#define subxx_ld(RG, RM, DP)                                                \
        subxz_ld(W(RG), W(RM), W(DP))

#define subxx_st(RG, RM, DP)                                                \
        subxz_st(W(RG), W(RM), W(DP))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))


#define subxz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define subxz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define subxz_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define subxz_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x2B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subxz_st(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x29)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define subxz_mr(RM, DP, RG)                                                \
        subxz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlxx_rx(RM)                     /* reads Recx for shift value */   \
        shlxz_rx(W(RM))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shlxz_mx(W(RM), W(DP))

#define shlxx_ri(RM, IM)                                                    \
        shlxz_ri(W(RM), W(IM))

#define shlxx_mi(RM, DP, IM)                                                \
        shlxz_mi(W(RM), W(DP), W(IM))

#define shlxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shlxz_rr(W(RG), W(RM))

#define shlxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shlxz_ld(W(RG), W(RM), W(DP))

#define shlxx_st(RG, RM, DP)                                                \
        shlxz_st(W(RG), W(RM), W(DP))

#define shlxx_mr(RM, DP, RG)                                                \
        shlxx_st(W(RG), W(RM), W(DP))


#define shlxz_rx(RM)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define shlxz_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shlxz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shlxz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shlxz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movxx_rr(Recx, W(RM))                                               \
        shlxz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlxz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movxx_ld(Recx, W(RM), W(DP))                                        \
        shlxz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shlxz_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movxx_rr(Recx, W(RG))                                               \
        shlxz_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shlxz_mr(RM, DP, RG)                                                \
        shlxz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrxx_rx(RM)                     /* reads Recx for shift value */   \
        shrxz_rx(W(RM))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift value */   \
        shrxz_mx(W(RM), W(DP))

#define shrxx_ri(RM, IM)                                                    \
        shrxz_ri(W(RM), W(IM))

#define shrxx_mi(RM, DP, IM)                                                \
        shrxz_mi(W(RM), W(DP), W(IM))

#define shrxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        shrxz_rr(W(RG), W(RM))

#define shrxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        shrxz_ld(W(RG), W(RM), W(DP))

#define shrxx_st(RG, RM, DP)                                                \
        shrxz_st(W(RG), W(RM), W(DP))

#define shrxx_mr(RM, DP, RG)                                                \
        shrxx_st(W(RG), W(RM), W(DP))


#define shrxz_rx(RM)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define shrxz_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrxz_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrxz_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shrxz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movxx_rr(Recx, W(RM))                                               \
        shrxz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrxz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movxx_ld(Recx, W(RM), W(DP))                                        \
        shrxz_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrxz_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movxx_rr(Recx, W(RG))                                               \
        shrxz_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shrxz_mr(RM, DP, RG)                                                \
        shrxz_st(W(RG), W(RM), W(DP))


#define shrxn_rx(RM)                     /* reads Recx for shift value */   \
        REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \

#define shrxn_mx(RM, DP)                 /* reads Recx for shift value */   \
    ADR REW(0,       RXB(RM)) EMITB(0xD3)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define shrxn_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMITB(VAL(IM) & 0x3F))

#define shrxn_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0xC1)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMITB(VAL(IM) & 0x3F))

#define shrxn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movxx_rr(Recx, W(RM))                                               \
        shrxn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrxn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        stack_st(Recx)                                                      \
        movxx_ld(Recx, W(RM), W(DP))                                        \
        shrxn_rx(W(RG))                                                     \
        stack_ld(Recx)

#define shrxn_st(RG, RM, DP)                                                \
        stack_st(Recx)                                                      \
        movxx_rr(Recx, W(RG))                                               \
        shrxn_mx(W(RM), W(DP))                                              \
        stack_ld(Recx)

#define shrxn_mr(RM, DP, RG)                                                \
        shrxn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulxx_ri(RM, IM)                                                    \
        REW(RXB(RM), RXB(RM)) EMITB(0x69 | TYP(IM))                         \
        MRM(REG(RM), MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define mulxx_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RM), REG(RM))

#define mulxx_ld(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x0F) EMITB(0xAF)                       \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x04,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x05,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxn_xr(W(RM))       /* product must not exceed operands size */

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxn_xm(W(RM), W(DP))/* product must not exceed operands size */

/* div
 * set-flags: undefined */

#define divxx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movxx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movxx_rr(Reax, W(RM))                                               \
        prexx_rr()                                                          \
        divxx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movxx_rr(W(RM), Reax)                                               \
        stack_ld(Reax)

#define divxx_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexx_rr()                                                          \
        divxx_xr(W(RM))                                                     \
        stack_ld(Redx)                                                      \
        movxx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divxx_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexx_rr()                                                          \
        divxx_xm(W(RM), W(DP))                                              \
        stack_ld(Redx)                                                      \
        movxx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divxn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movxx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movxx_rr(Reax, W(RM))                                               \
        prexn_rr()                                                          \
        divxn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movxx_rr(W(RM), Reax)                                               \
        stack_ld(Reax)

#define divxn_rr(RG, RM)                 /* RG, RM no Reax, RM no Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexn_rr()                                                          \
        divxn_xr(W(RM))                                                     \
        stack_ld(Redx)                                                      \
        movxx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divxn_ld(RG, RM, DP)   /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexn_rr()                                                          \
        divxn_xm(W(RM), W(DP))                                              \
        stack_ld(Redx)                                                      \
        movxx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
        movxx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrxn_ri(Redx, IC(63))


#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x06,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   EMPTY)

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
    ADR REW(0,       RXB(RM)) EMITB(0xF7)                                   \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)


#define divxp_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xr(W(RM))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divxp_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divxn_xm(W(RM), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remxx_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movxx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movxx_rr(Reax, W(RM))                                               \
        prexx_rr()                                                          \
        divxx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movxx_rr(W(RM), Redx)                                               \
        stack_ld(Redx)

#define remxx_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexx_rr()                                                          \
        divxx_xr(W(RM))                                                     \
        stack_ld(Reax)                                                      \
        movxx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remxx_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexx_rr()                                                          \
        divxx_xm(W(RM), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movxx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remxn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movxx_mi(Mebp, inf_SCR01(0), W(IM))                                 \
        movxx_rr(Reax, W(RM))                                               \
        prexn_rr()                                                          \
        divxn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movxx_rr(W(RM), Redx)                                               \
        stack_ld(Redx)

#define remxn_rr(RG, RM)                 /* RG, RM no Redx, RM no Reax */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexn_rr()                                                          \
        divxn_xr(W(RM))                                                     \
        stack_ld(Reax)                                                      \
        movxx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define remxn_ld(RG, RM, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movxx_rr(Reax, W(RG))                                               \
        prexn_rr()                                                          \
        divxn_xm(W(RM), W(DP))                                              \
        stack_ld(Reax)                                                      \
        movxx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
                                     /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
                                     /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj
 * set-flags: undefined
 * refer to individual instructions' description
 * to stay within special register limitations */

#define arjxx_rx(RM, op, cc, lb)                                            \
        AR1(W(RM), op, xz_rx)                                               \
        CMJ(cc, lb)

#define arjxx_mx(RM, DP, op, cc, lb)                                        \
        AR2(W(RM), W(DP), op, xz_mx)                                        \
        CMJ(cc, lb)

#define arjxx_ri(RM, IM, op, cc, lb)                                        \
        AR2(W(RM), W(IM), op, xz_ri)                                        \
        CMJ(cc, lb)

#define arjxx_mi(RM, DP, IM, op, cc, lb)                                    \
        AR3(W(RM), W(DP), W(IM), op, xz_mi)                                 \
        CMJ(cc, lb)

#define arjxx_rr(RG, RM, op, cc, lb)                                        \
        AR2(W(RG), W(RM), op, xz_rr)                                        \
        CMJ(cc, lb)

#define arjxx_ld(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, xz_ld)                                 \
        CMJ(cc, lb)

#define arjxx_st(RG, RM, DP, op, cc, lb)                                    \
        AR3(W(RG), W(RM), W(DP), op, xz_st)                                 \
        CMJ(cc, lb)

#define arjxx_mr(RM, DP, RG, op, cc, lb)                                    \
        arjxx_st(W(RG), W(RM), W(DP), op, cc, lb)

/* cmj
 * set-flags: undefined */

#define cmjxx_rz(RM, cc, lb)                                                \
        cmjxx_ri(W(RM), IC(0), cc, lb)

#define cmjxx_mz(RM, DP, cc, lb)                                            \
        cmjxx_mi(W(RM), W(DP), IC(0), cc, lb)

#define cmjxx_ri(RM, IM, cc, lb)                                            \
        cmpxx_ri(W(RM), W(IM))                                              \
        CMJ(cc, lb)

#define cmjxx_mi(RM, DP, IM, cc, lb)                                        \
        cmpxx_mi(W(RM), W(DP), W(IM))                                       \
        CMJ(cc, lb)

#define cmjxx_rr(RG, RM, cc, lb)                                            \
        cmpxx_rr(W(RG), W(RM))                                              \
        CMJ(cc, lb)

#define cmjxx_rm(RG, RM, DP, cc, lb)                                        \
        cmpxx_rm(W(RG), W(RM), W(DP))                                       \
        CMJ(cc, lb)

#define cmjxx_mr(RM, DP, RG, cc, lb)                                        \
        cmpxx_mr(W(RM), W(DP), W(RG))                                       \
        CMJ(cc, lb)

/* cmp
 * set-flags: yes */

#define cmpxx_ri(RM, IM)                                                    \
        REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(EMPTY,   EMPTY,   CMD(IM))

#define cmpxx_mi(RM, DP, IM)                                                \
    ADR REW(0,       RXB(RM)) EMITB(0x81 | TYP(IM))                         \
        MRM(0x07,    MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), CMD(IM))

#define cmpxx_rr(RG, RM)                                                    \
        REW(RXB(RG), RXB(RM)) EMITB(0x3B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))

#define cmpxx_rm(RG, RM, DP)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x3B)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#define cmpxx_mr(RM, DP, RG)                                                \
    ADR REW(RXB(RG), RXB(RM)) EMITB(0x39)                                   \
        MRM(REG(RG), MOD(RM), REG(RM))                                      \
        AUX(SIB(RM), CMD(DP), EMPTY)

#endif /* RT_RTARCH_X64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

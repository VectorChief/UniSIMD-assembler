/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_H
#define RT_RTARCH_A64_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64.h: Implementation of AArch64:ARMv8 BASE instructions.
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

#include "rtarch_a32.h"

/******************************************************************************/
/**********************************   A64   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RM), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xF9000000 | MDM(TIxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define movxx_rr(RG, RM)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), TZxx,    REG(RM)))

#define movxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(REG(RG), MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define movxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9000000 | MDM(REG(RG), MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x80000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x80000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define andxx_rr(RG, RM)                                                    \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define andxz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xE0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define andxz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define andxz_rr(RG, RM)                                                    \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define andxz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), TMxx))

#define andxz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xA0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define orrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define orrxx_rr(RG, RM)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define orrxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define orrxz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xA0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF1000000 | REG(RM) << 5)               /* <- set flags (Z) */

#define orrxz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrxz_rr(RG, RM)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrxz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrxz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xC0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))

#define xorxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xC0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define xorxx_rr(RG, RM)                                                    \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define xorxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define xorxz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0xC0000000 | MIM(REG(RM), REG(RM), VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF1000000 | REG(RM) << 5)               /* <- set flags (Z) */

#define xorxz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xC0000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorxz_rr(RG, RM)                                                    \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorxz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorxz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

/* not
 * set-flags: no */

#define notxx_rx(RM)                                                        \
        EMITW(0xAA200000 | MRM(REG(RM), TZxx,    REG(RM)))

#define notxx_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negxx_rx(RM)                                                        \
        EMITW(0xCB000000 | MRM(REG(RM), TZxx,    REG(RM)))

#define negxx_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define negxz_rx(RM)                                                        \
        EMITW(0xEB000000 | MRM(REG(RM), TZxx,    REG(RM)))

#define negxz_mx(RM, DP)                                                    \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x81000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x81000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define addxx_rr(RG, RM)                                                    \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8B000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))


#define addxz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xA1000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define addxz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA1000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define addxz_rr(RG, RM)                                                    \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define addxz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), TMxx))

#define addxz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAB000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xC1000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xC1000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subxx_rr(RG, RM)                                                    \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCB000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subxx_mr(RM, DP, RG)                                                \
        subxx_st(W(RG), W(RM), W(DP))


#define subxz_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE1000000 | MIM(REG(RM), REG(RM), VAL(IM), T1(IM), M1(IM)))

#define subxz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subxz_rr(RG, RM)                                                    \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), REG(RM)))

#define subxz_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subxz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define subxz_mr(RM, DP, RG)                                                \
        subxz_st(W(RG), W(RM), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlxx_rx(RM)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02000 | MRM(REG(RM), REG(RM), Tecx))

#define shlxx_mx(RM, DP)                 /* reads Recx for shift count */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shlxx_ri(RM, IM)                                                    \
        EMITW(0xD3400000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (-VAL(IM)&0x3F)<<16 | (63-(VAL(IM)&0x3F))<<10)

#define shlxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD3400000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IM)&0x3F)<<16 | (63-(VAL(IM)&0x3F))<<10)                      \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shlxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), REG(RM)))

#define shlxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shlxx_mr(RM, DP, RG)                                                \
        shlxx_st(W(RG), W(RM), W(DP))


#define shlxz_rx(RM)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02000 | MRM(REG(RM), REG(RM), Tecx))                     \
        EMITW(0xF1000000 | REG(RM) << 5)               /* <- set flags (Z) */

#define shlxz_mx(RM, DP)                 /* reads Recx for shift count */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlxz_ri(RM, IM)                                                    \
        EMITW(0xD3400000 | MRM(REG(RM), REG(RM), 0x00) |                    \
        (-VAL(IM)&0x3F)<<16 | (63-(VAL(IM)&0x3F))<<10)                      \
        EMITW(0xF1000000 | REG(RM) << 5)               /* <- set flags (Z) */

#define shlxz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD3400000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IM)&0x3F)<<16 | (63-(VAL(IM)&0x3F))<<10)                      \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlxz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlxz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlxz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlxz_mr(RM, DP, RG)                                                \
        shlxz_st(W(RG), W(RM), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrxx_rx(RM)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02400 | MRM(REG(RM), REG(RM), Tecx))

#define shrxx_mx(RM, DP)                 /* reads Recx for shift count */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxx_ri(RM, IM)                                                    \
        EMITW(0xD340FC00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x3F)<<16)

#define shrxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxx_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), REG(RM)))

#define shrxx_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrxx_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxx_mr(RM, DP, RG)                                                \
        shrxx_st(W(RG), W(RM), W(DP))


#define shrxz_rx(RM)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02400 | MRM(REG(RM), REG(RM), Tecx))                     \
        EMITW(0xF1000000 | REG(RM) << 5)               /* <- set flags (Z) */

#define shrxz_mx(RM, DP)                 /* reads Recx for shift count */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrxz_ri(RM, IM)                                                    \
        EMITW(0xD340FC00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x3F)<<16) \
        EMITW(0xF1000000 | REG(RM) << 5)               /* <- set flags (Z) */

#define shrxz_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrxz_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), REG(RM)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrxz_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrxz_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrxz_mr(RM, DP, RG)                                                \
        shrxz_st(W(RG), W(RM), W(DP))


#define shrxn_rx(RM)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02800 | MRM(REG(RM), REG(RM), Tecx))

#define shrxn_mx(RM, DP)                 /* reads Recx for shift count */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxn_ri(RM, IM)                                                    \
        EMITW(0x9340FC00 | MRM(REG(RM), REG(RM), 0x00) |(VAL(IM)&0x3F)<<16)

#define shrxn_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IM)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxn_rr(RG, RM)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), REG(RM)))

#define shrxn_ld(RG, RM, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrxn_st(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    REG(RG)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))

#define shrxn_mr(RM, DP, RG)                                                \
        shrxn_st(W(RG), W(RM), W(DP))

/* mul
 * set-flags: undefined */

#define mulxx_ri(RM, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x9B007C00 | MRM(REG(RM), REG(RM), TIxx))

#define mulxx_rr(RG, RM)                                                    \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define mulxx_ld(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulxx_xr(RM)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RM)))

#define mulxx_xm(RM, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))


#define mulxn_xr(RM)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    REG(RM)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RM)))

#define mulxn_xm(RM, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))


#define mulxp_xr(RM)     /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxx_rr(Reax, W(RM)) /* product must not exceed operands size */

#define mulxp_xm(RM, DP) /* Reax is in/out, prepares Redx for divxn_x* */   \
        mulxx_ld(Reax, W(RM), W(DP))  /* must not exceed operands size */

/* div
 * set-flags: undefined */

#define divxx_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x9AC00800 | MRM(REG(RM), REG(RM), TIxx))

#define divxx_rr(RG, RM)                /* RG no Reax, RM no Reax/Redx */   \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), REG(RM)))

#define divxx_ld(RG, RM, DP)            /* RG no Reax, RM no Oeax/Medx */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divxn_ri(RM, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x9AC00C00 | MRM(REG(RM), REG(RM), TIxx))

#define divxn_rr(RG, RM)                /* RG no Reax, RM no Reax/Redx */   \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), REG(RM)))

#define divxn_ld(RG, RM, DP)            /* RG no Reax, RM no Oeax/Medx */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prexx_xx()          /* to be placed immediately prior divxx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prexn_xx()          /* to be placed immediately prior divxn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divxx_xr(RM)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    REG(RM)))

#define divxx_xm(RM, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    TMxx))


#define divxn_xr(RM)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    REG(RM)))

#define divxn_xm(RM, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    TMxx))


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
        movxx_rr(Redx, W(RM))                                               \
        divxx_ri(W(RM), W(IM))                                              \
        EMITW(0x9B008000 | MRM(REG(RM), REG(RM), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remxx_rr(RG, RM)                /* RG no Redx, RM no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxx_rr(W(RG), W(RM))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RM)) | Tedx << 10)     \
        stack_ld(Redx)

#define remxx_ld(RG, RM, DP)            /* RG no Redx, RM no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxx_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remxn_ri(RM, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RM))                                               \
        divxn_ri(W(RM), W(IM))                                              \
        EMITW(0x9B008000 | MRM(REG(RM), REG(RM), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remxn_rr(RG, RM)                /* RG no Redx, RM no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxn_rr(W(RG), W(RM))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RM)) | Tedx << 10)     \
        stack_ld(Redx)

#define remxn_ld(RG, RM, DP)            /* RG no Redx, RM no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movxx_rr(Redx, W(RG))                                               \
        divxn_ld(W(RG), W(RM), W(DP))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remxx_xx()          /* to be placed immediately prior divxx_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxx_xr(RM)        /* to be placed immediately after divxx_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remxx_xm(RM, DP)    /* to be placed immediately after divxx_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remxn_xx()          /* to be placed immediately prior divxn_x* */   \
        movxx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remxn_xr(RM)        /* to be placed immediately after divxn_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RM)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remxn_xm(RM, DP)    /* to be placed immediately after divxn_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

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
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0xE1000000 | MIM(TZxx,    REG(RM), VAL(IM), T1(IM), M1(IM)))

#define cmpxx_mi(RM, DP, IM)                                                \
        AUW(SIB(RM),  VAL(IM), TIxx,    MOD(RM), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1000000 | MIM(TZxx,    TMxx,    VAL(IM), T1(IM), M1(IM)))

#define cmpxx_rr(RG, RM)                                                    \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RG), REG(RM)))

#define cmpxx_rm(RG, RM, DP)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RG), TMxx))

#define cmpxx_mr(RM, DP, RG)                                                \
        AUW(SIB(RM),  EMPTY,  EMPTY,    MOD(RM), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(RM), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TZxx,    TMxx,    REG(RG)))

#endif /* RT_RTARCH_A64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

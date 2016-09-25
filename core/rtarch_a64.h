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
 * DP - displacement value (of given size DP, DF, DG, DH, DV)
 * IS - immediate value (is used as a second or first source)
 * IT - immediate value (is used as a third or second source)
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

#define movzx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movzx_mi(MD, DP, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DP), C1(DP), G3(IS))   \
        EMITW(0xF9000000 | MDM(TIxx,    MOD(MD), VXL(DP), B1(DP), P1(DP)))

#define movzx_rr(RD, RS)                                                    \
        EMITW(0xAA000000 | MRM(REG(RD), TZxx,    REG(RS)))

#define movzx_ld(RD, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(REG(RD), MOD(MS), VXL(DP), B1(DP), P1(DP)))

#define movzx_st(RS, MD, DP)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9000000 | MDM(REG(RS), MOD(MD), VXL(DP), B1(DP), P1(DP)))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x80000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x80000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define andzx_rr(RG, RS)                                                    \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))


#define andzz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define andzz_rr(RG, RS)                                                    \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), TMxx))

#define andzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annzx_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzx_ri(W(RG), W(IS))

#define annzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x80000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzx_rr(RG, RS)                                                    \
        EMITW(0x8A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzx_mr(MG, DP, RS)                                                \
        annzx_st(W(RS), W(MG), W(DP))


#define annzz_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzz_ri(W(RG), W(IS))

#define annzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzz_rr(RG, RS)                                                    \
        EMITW(0xEA200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEA200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzz_mr(MG, DP, RS)                                                \
        annzz_st(W(RS), W(MG), W(DP))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xA0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define orrzx_rr(RG, RS)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))


#define orrzz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xA0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrzz_rr(RG, RS)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornzx_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzx_ri(W(RG), W(IS))

#define ornzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define ornzx_rr(RG, RS)                                                    \
        EMITW(0xAA200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define ornzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define ornzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define ornzx_mr(MG, DP, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DP))


#define ornzz_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzz_ri(W(RG), W(IS))

#define ornzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornzz_rr(RG, RS)                                                    \
        EMITW(0xAA200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornzz_mr(MG, DP, RS)                                                \
        ornzz_st(W(RS), W(MG), W(DP))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xC0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xC0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define xorzx_rr(RG, RS)                                                    \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))


#define xorzz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xC0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xC0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorzz_rr(RG, RS)                                                    \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

/* not
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        EMITW(0xAA200000 | MRM(REG(RG), TZxx,    REG(RG)))

#define notzx_mx(MG, DP)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negzx_rx(RG)                                                        \
        EMITW(0xCB000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define negzx_mx(MG, DP)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))


#define negzz_rx(RG)                                                        \
        EMITW(0xEB000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define negzz_mx(MG, DP)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x81000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x81000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define addzx_rr(RG, RS)                                                    \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x8B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))


#define addzz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xA1000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xA1000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define addzz_rr(RG, RS)                                                    \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), TMxx))

#define addzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xAB000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xC1000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xC1000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define subzx_rr(RG, RS)                                                    \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xCB000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define subzx_mr(MG, DP, RS)                                                \
        subzx_st(W(RS), W(MG), W(DP))


#define subzz_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE1000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IS))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define subzz_rr(RG, RS)                                                    \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define subzz_mr(MG, DP, RS)                                                \
        subzz_st(W(RS), W(MG), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlzx_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), Tecx))

#define shlzx_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shlzx_ri(RG, IS)                                                    \
        EMITW(0xD3400000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)

#define shlzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD3400000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)                      \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), REG(RS)))

#define shlzx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shlzx_mr(MG, DP, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DP))


#define shlzz_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzz_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlzz_ri(RG, IS)                                                    \
        EMITW(0xD3400000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)                      \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD3400000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)                      \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlzz_mr(MG, DP, RS)                                                \
        shlzz_st(W(RS), W(MG), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrzx_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), Tecx))

#define shrzx_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shrzx_ri(RG, IS)                                                    \
        EMITW(0xD340FC00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x3F)<<16)

#define shrzx_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrzx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shrzx_mr(MG, DP, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DP))


#define shrzz_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzz_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrzz_ri(RG, IS)                                                    \
        EMITW(0xD340FC00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzz_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xD340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrzz_mr(MG, DP, RS)                                                \
        shrzz_st(W(RS), W(MG), W(DP))


#define shrzn_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), Tecx))

#define shrzn_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shrzn_ri(RG, IS)                                                    \
        EMITW(0x9340FC00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x3F)<<16)

#define shrzn_mi(MG, DP, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrzn_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrzn_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF9000000 | MDM(TMxx,    MOD(MG), VXL(DP), B1(DP), P1(DP)))

#define shrzn_mr(MG, DP, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DP))

/* mul
 * set-flags: undefined */

#define mulzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TIxx))

#define mulzx_rr(RG, RS)                                                    \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RS)))

#define mulzx_xm(MS, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RS)))

#define mulzn_xm(MS, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))


#define mulzp_xr(RS)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulzp_xm(MS, DP) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_ld(Reax, W(MS), W(DP))  /* must not exceed operands size */

/* div
 * set-flags: undefined */

#define divzx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divzx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), REG(RS)))

#define divzx_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divzn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divzn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define divzn_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prezx_xx()          /* to be placed immediately prior divzx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prezn_xx()          /* to be placed immediately prior divzn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divzx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    REG(RS)))

#define divzx_xm(MS, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    TMxx))


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    REG(RS)))

#define divzn_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    TMxx))


#define divzp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divzp_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DP))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem
 * set-flags: undefined */

#define remzx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_ri(W(RG), W(IS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remzx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_rr(W(RG), W(RS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remzx_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_ld(W(RG), W(MS), W(DP))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remzn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ri(W(RG), W(IS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remzn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_rr(W(RG), W(RS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remzn_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ld(W(RG), W(MS), W(DP))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remzx_xm(MS, DP)    /* to be placed immediately after divzx_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remzn_xm(MS, DP)    /* to be placed immediately after divzn_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

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

#define arjzx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, zz_ri)                                        \
        CMJ(cc, lb)

#define arjzx_mi(MG, DP, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DP), W(IS), op, zz_mi)                                 \
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

#define cmjzx_ri(RS, IT, cc, lb)                                            \
        cmpzx_ri(W(RS), W(IT))                                              \
        CMJ(cc, lb)

#define cmjzx_mi(MS, DP, IT, cc, lb)                                        \
        cmpzx_mi(W(MS), W(DP), W(IT))                                       \
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

#define cmpzx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IT))   \
        EMITW(0xE1000000 | MIM(TZxx,    REG(RS), VAL(IT), T1(IT), M1(IT)))

#define cmpzx_mi(MS, DP, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DP), C1(DP), G1(IT))   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xE1000000 | MIM(TZxx,    TMxx,    VAL(IT), T1(IT), M1(IT)))

#define cmpzx_rr(RS, RT)                                                    \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RS), REG(RT)))

#define cmpzx_rm(RS, MT, DP)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MT), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RS), TMxx))

#define cmpzx_mr(MS, DP, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF9400000 | MDM(TMxx,    MOD(MS), VXL(DP), B1(DP), P1(DP)))  \
        EMITW(0xEB000000 | MRM(TZxx,    TMxx,    REG(RT)))

#endif /* RT_RTARCH_A64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2021 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_A64_H
#define RT_RTARCH_A64_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_a64.h: Implementation of AArch64 64-bit BASE instructions.
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
 * Setting-flags instruction naming scheme has been changed again recently for
 * better orthogonality with operand size, type and args-list. It is therefore
 * recommended to use combined-arithmetic-jump (arj) for better API stability
 * and maximum efficiency across all supported targets. For similar reasons
 * of higher performance on MIPS and POWER use combined-compare-jump (cmj).
 * Not all canonical forms of BASE instructions have efficient implementation.
 * For example, some forms of shifts and division use stack ops on x86 targets,
 * while standalone remainders can only be done natively on MIPSr6 and POWER9.
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

#include "rtarch_a32.h"

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movzx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movzx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TIxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0xF8000000 | MDM(TIxx,    MOD(MD), VXL(DD), B1(DD), P1(DD)))

#define movzx_rr(RD, RS)                                                    \
        EMITW(0xAA000000 | MRM(REG(RD), TZxx,    REG(RS)))

#define movwn_rr(RD, RS)       /* move 32-bit to 64-bit with sign-extend */ \
        EMITW(0x93407C00 | MRM(REG(RD), REG(RS), 0x00))

#define movwz_rr(RD, RS)       /* move 32-bit to 64-bit with zero-extend */ \
        EMITW(0x2A000000 | MRM(REG(RD), TZxx,    REG(RS)))

#define movzx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(REG(RD), MOD(MS), VXL(DS), B1(DS), P1(DS)))

#define movwn_ld(RD, MS, DS)   /* load 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8800000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movwz_ld(RD, MS, DS)   /* load 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8400000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), P1(DS)))

#define movzx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0xF8000000 | MDM(REG(RS), MOD(MD), VXL(DD), B1(DD), P1(DD)))


#define movzx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        AUW(EMPTY,    VAL(IT), RXG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IT))   \

#define movzx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        AUW(EMPTY,    VAL(IS), TMxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        AUW(SIB(MD),  VAL(IT), TMxx+32, MOD(MD), VAL(DD), C1(DD), G3(IT))   \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MD), VXL(DD), B1(DD), P1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x80000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x80000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define andzx_rr(RG, RS)                                                    \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x8A000000 | MRM(REG(RG), REG(RG), TMxx))

#define andzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x8A000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define andzx_mr(MG, DG, RS)                                                \
        andzx_st(W(RS), W(MG), W(DG))


#define andzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xE0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define andzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define andzxZrr(RG, RS)                                                    \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define andzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEA000000 | MRM(REG(RG), REG(RG), TMxx))

#define andzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xEA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define andzxZmr(MG, DG, RS)                                                \
        andzxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annzx_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzx_ri(W(RG), W(IS))

#define annzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0x80000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define annzx_rr(RG, RS)                                                    \
        EMITW(0x8A200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x8A200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x8A200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define annzx_mr(MG, DG, RS)                                                \
        annzx_st(W(RS), W(MG), W(DG))


#define annzxZri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzxZri(W(RG), W(IS))

#define annzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xE0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define annzxZrr(RG, RS)                                                    \
        EMITW(0xEA200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define annzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEA200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define annzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xEA200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define annzxZmr(MG, DG, RS)                                                \
        annzxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xA0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define orrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define orrzx_rr(RG, RS)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define orrzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), TMxx))

#define orrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define orrzx_mr(MG, DG, RS)                                                \
        orrzx_st(W(RS), W(MG), W(DG))


#define orrzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xA0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrzxZrr(RG, RS)                                                    \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xAA000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define orrzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define orrzxZmr(MG, DG, RS)                                                \
        orrzxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornzx_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzx_ri(W(RG), W(IS))

#define ornzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define ornzx_rr(RG, RS)                                                    \
        EMITW(0xAA200000 | MRM(REG(RG), REG(RS), REG(RG)))

#define ornzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xAA200000 | MRM(REG(RG), TMxx,    REG(RG)))

#define ornzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define ornzx_mr(MG, DG, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DG))


#define ornzxZri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzxZri(W(RG), W(IS))

#define ornzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xA0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornzxZrr(RG, RS)                                                    \
        EMITW(0xAA200000 | MRM(REG(RG), REG(RS), REG(RG)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xAA200000 | MRM(REG(RG), TMxx,    REG(RG)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define ornzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA200000 | MRM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define ornzxZmr(MG, DG, RS)                                                \
        ornzxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xC0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))

#define xorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xC0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define xorzx_rr(RG, RS)                                                    \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define xorzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), TMxx))

#define xorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xCA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define xorzx_mr(MG, DG, RS)                                                \
        xorzx_st(W(RS), W(MG), W(DG))


#define xorzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0xC0000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xC0000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorzxZrr(RG, RS)                                                    \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xCA000000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define xorzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xCA000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define xorzxZmr(MG, DG, RS)                                                \
        xorzxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        EMITW(0xAA200000 | MRM(REG(RG), TZxx,    REG(RG)))

#define notzx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAA200000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negzx_rx(RG)                                                        \
        EMITW(0xCB000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define negzx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xCB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))


#define negzxZrx(RG)                                                        \
        EMITW(0xEB000000 | MRM(REG(RG), TZxx,    REG(RG)))

#define negzxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TZxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x81000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x81000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define addzx_rr(RG, RS)                                                    \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addwn_ld(RG, MS, DS)    /* add 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8800000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addwz_ld(RG, MS, DS)    /* add 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x8B000000 | MRM(REG(RG), REG(RG), TMxx))

#define addzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x8B000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define addzx_mr(MG, DG, RS)                                                \
        addzx_st(W(RS), W(MG), W(DG))


#define addzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xA1000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define addzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA1000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define addzxZrr(RG, RS)                                                    \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), TMxx))

#define addwnZld(RG, MS, DS)    /* add 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8800000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), TMxx))

#define addwzZld(RG, MS, DS)    /* add 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xAB000000 | MRM(REG(RG), REG(RG), TMxx))

#define addzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xAB000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define addzxZmr(MG, DG, RS)                                                \
        addzxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xC1000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xC1000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define subzx_rr(RG, RS)                                                    \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subwn_ld(RG, MS, DS)  /* sub 32-bit from 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8800000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subwz_ld(RG, MS, DS)  /* sub 32-bit from 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xCB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xCB000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define subzx_mr(MG, DG, RS)                                                \
        subzx_st(W(RS), W(MG), W(DG))


#define subzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0xE1000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)))

#define subzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xE1000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)))  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define subzxZrr(RG, RS)                                                    \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subwnZld(RG, MS, DS)  /* sub 32-bit from 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8800000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subwzZld(RG, MS, DS)  /* sub 32-bit from 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEB000000 | MRM(REG(RG), REG(RG), TMxx))

#define subzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xEB000000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define subzxZmr(MG, DG, RS)                                                \
        subzxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), Tecx))

#define shlzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shlzx_ri(RG, IS)                                                    \
        EMITW(0xD3400000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)

#define shlzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xD3400000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)                      \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), REG(RS)))

#define shlzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), TMxx))

#define shlzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shlzx_mr(MG, DG, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DG))


#define shlzxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlzxZri(RG, IS)                                                    \
        EMITW(0xD3400000 | MRM(REG(RG), REG(RG), 0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)                      \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xD3400000 | MRM(TMxx,    TMxx,    0x00) |                    \
        (-VAL(IS)&0x3F)<<16 | (63-(VAL(IS)&0x3F))<<10)                      \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02000 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shlzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02000 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shlzxZmr(MG, DG, RS)                                                \
        shlzxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), Tecx))

#define shrzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shrzx_ri(RG, IS)                                                    \
        EMITW(0xD340FC00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x3F)<<16)

#define shrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xD340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), TMxx))

#define shrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shrzx_mr(MG, DG, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DG))


#define shrzxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrzxZri(RG, IS)                                                    \
        EMITW(0xD340FC00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xD340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02400 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02400 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrzxZmr(MG, DG, RS)                                                \
        shrzxZst(W(RS), W(MG), W(DG))


#define shrzn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), Tecx))

#define shrzn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shrzn_ri(RG, IS)                                                    \
        EMITW(0x9340FC00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x3F)<<16)

#define shrzn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), REG(RS)))

#define shrzn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), TMxx))

#define shrzn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define shrzn_mr(MG, DG, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DG))


#define shrznZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrznZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrznZri(RG, IS)                                                    \
        EMITW(0x9340FC00 | MRM(REG(RG), REG(RG), 0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrznZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9340FC00 | MRM(TMxx,    TMxx,    0x00) |(VAL(IS)&0x3F)<<16) \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrznZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrznZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02800 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define shrznZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02800 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define shrznZmr(MG, DG, RS)                                                \
        shrznZst(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 64 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02C00 | MRM(REG(RG), REG(RG), Tecx))

#define rorzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02C00 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define rorzx_ri(RG, IS)                                                    \
        EMITW(0x93C00000 | MRM(REG(RG), REG(RG), REG(RG)) |                 \
                                        (VAL(IS)&0x3F)<<10)

#define rorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x93C00000 | MRM(TMxx,    TMxx,    TMxx) |(VAL(IS)&0x3F)<<10) \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define rorzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define rorzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02C00 | MRM(REG(RG), REG(RG), TMxx))

#define rorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02C00 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))

#define rorzx_mr(MG, DG, RS)                                                \
        rorzx_st(W(RS), W(MG), W(DG))


#define rorzxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x9AC02C00 | MRM(REG(RG), REG(RG), Tecx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02C00 | MRM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define rorzxZri(RG, IS)                                                    \
        EMITW(0x93C00000 | MRM(REG(RG), REG(RG), REG(RG)) |                 \
                                        (VAL(IS)&0x3F)<<10)                 \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x93C00000 | MRM(TMxx,    TMxx,    TMxx) |(VAL(IS)&0x3F)<<10) \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define rorzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x9AC02C00 | MRM(REG(RG), REG(RG), REG(RS)))                  \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC02C00 | MRM(REG(RG), REG(RG), TMxx))                     \
        EMITW(0xF1000000 | REG(RG) << 5)               /* <- set flags (Z) */

#define rorzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0x9AC02C00 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VXL(DG), B1(DG), P1(DG)))  \
        EMITW(0xF1000000 | TMxx << 5)                  /* <- set flags (Z) */

#define rorzxZmr(MG, DG, RS)                                                \
        rorzxZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TIxx))

#define mulzx_rr(RG, RS)                                                    \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TMxx))

#define mulwn_ld(RG, MS, DS)  /* mul 64-bit with 32-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8800000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TMxx))

#define mulwz_ld(RG, MS, DS)  /* mul 64-bit with 32-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9B007C00 | MRM(REG(RG), REG(RG), TMxx))


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RS)))

#define mulzx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9BC07C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    REG(RS)))

#define mulzn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9B407C00 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x9B007C00 | MRM(Teax,    Teax,    TMxx))


#define mulzp_xr(RS)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulzp_xm(MS, DS) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divzx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), TIxx))

#define divzx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), REG(RS)))

#define divzx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC00800 | MRM(REG(RG), REG(RG), TMxx))


#define divzn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), TIxx))

#define divzn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), REG(RS)))

#define divzn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC00C00 | MRM(REG(RG), REG(RG), TMxx))


#define prezx_xx()          /* to be placed immediately prior divzx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prezn_xx()          /* to be placed immediately prior divzn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divzx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    REG(RS)))

#define divzx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC00800 | MRM(Teax,    Teax,    TMxx))


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    REG(RS)))

#define divzn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0x9AC00C00 | MRM(Teax,    Teax,    TMxx))


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
        movzx_rr(Redx, W(RG))                                               \
        divzx_ri(W(RG), W(IS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remzx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_rr(W(RG), W(RS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remzx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remzn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ri(W(RG), W(IS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TIxx) | Tedx << 10)        \
        stack_ld(Redx)

#define remzn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_rr(W(RG), W(RS))                                              \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), REG(RS)) | Tedx << 10)     \
        stack_ld(Redx)

#define remzn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x9B008000 | MRM(REG(RG), REG(RG), TMxx) | Tedx << 10)        \
        stack_ld(Redx)


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remzx_xm(MS, DS)    /* to be placed immediately after divzx_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    REG(RS)) | Tedx << 10)     \
                                                          /* Redx<-rem */

#define remzn_xm(MS, DS)    /* to be placed immediately after divzn_xm */   \
        EMITW(0x9B008000 | MRM(Tedx,    Teax,    TMxx) | Tedx << 10)        \
                                                          /* Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjzx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, z, Zrx)                                              \
        CMJ(cc, lb)

#define arjzx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, z, Zmx)                                       \
        CMJ(cc, lb)

#define arjzx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, z, Zri)                                       \
        CMJ(cc, lb)

#define arjzx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, z, Zmi)                                \
        CMJ(cc, lb)

#define arjzx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, z, Zrr)                                       \
        CMJ(cc, lb)

#define arjzx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, z, Zld)                                \
        CMJ(cc, lb)

#define arjzx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, z, Zst)                                \
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

#define cmjwn_rm(RS, MT, DT, cc, lb)   /* cmj 64/32-bit with sign-extend */ \
        cmpwn_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjwz_rm(RS, MT, DT, cc, lb)   /* cmj 64/32-bit with zero-extend */ \
        cmpwz_rm(W(RS), W(MT), W(DT))                                       \
        CMJ(cc, lb)

#define cmjzx_mr(MS, DS, RT, cc, lb)                                        \
        cmpzx_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjwn_mr(MS, DS, RT, cc, lb)   /* cmj 32/64-bit with sign-extend */ \
        cmpwn_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

#define cmjwz_mr(MS, DS, RT, cc, lb)   /* cmj 32/64-bit with zero-extend */ \
        cmpwz_mr(W(MS), W(DS), W(RT))                                       \
        CMJ(cc, lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpzx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IT))   \
        EMITW(0xE1000000 | MIM(TZxx,    REG(RS), VAL(IT), T1(IT), M1(IT)))

#define cmpzx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TIxx,    MOD(MS), VAL(DS), C1(DS), G1(IT))   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xE1000000 | MIM(TZxx,    TMxx,    VAL(IT), T1(IT), M1(IT)))

#define cmpzx_rr(RS, RT)                                                    \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RS), REG(RT)))

#define cmpzx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MT), VXL(DT), B1(DT), P1(DT)))  \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RS), TMxx))

#define cmpwn_rm(RS, MT, DT)    /* cmp 64-bit to 32-bit with sign-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xB8800000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RS), TMxx))

#define cmpwz_rm(RS, MT, DT)    /* cmp 64-bit to 32-bit with zero-extend */ \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0xB8400000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(0xEB000000 | MRM(TZxx,    REG(RS), TMxx))

#define cmpzx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xF8400000 | MDM(TMxx,    MOD(MS), VXL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEB000000 | MRM(TZxx,    TMxx,    REG(RT)))

#define cmpwn_mr(MS, DS, RT)    /* cmp 32-bit to 64-bit with sign-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8800000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEB000000 | MRM(TZxx,    TMxx,    REG(RT)))

#define cmpwz_mr(MS, DS, RT)    /* cmp 32-bit to 64-bit with zero-extend */ \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0xB8400000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xEB000000 | MRM(TZxx,    TMxx,    REG(RT)))

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
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_A64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

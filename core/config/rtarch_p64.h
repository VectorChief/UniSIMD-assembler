/******************************************************************************/
/* Copyright (c) 2013-2020 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P64_H
#define RT_RTARCH_P64_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p64.h: Implementation of POWER 64-bit BASE instructions.
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

#include "rtarch_p32.h"

#if (defined RT_P64)

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/* mov (D = S)
 * set-flags: no */

#define movzx_ri(RD, IS)                                                    \
        AUW(EMPTY,    VAL(IS), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movzx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  VAL(IS), TWxx,    MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MD), VAL(DD), B1(DD), Q1(DD)))

#define movzx_rr(RD, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RD), REG(RS), REG(RS)))

#define movzx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RD), MOD(MS), VAL(DS), B1(DS), F1(DS)))

#define movzx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DD), C1(DD), EMPTY2)   \
        EMITW(0x00000000 | MDM(REG(RS), MOD(MD), VAL(DD), B1(DD), Q1(DD)))


#define movzx_rj(RD, IT, IS)     /* IT - upper 32-bit, IS - lower 32-bit */ \
        AUW(EMPTY,    VAL(IT), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x780007C6 | MSM(REG(RD), REG(RD), 0x00))                     \
        AUW(EMPTY,    VAL(IS), RXG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define movzx_mj(MD, DD, IT, IS) /* IT - upper 32-bit, IS - lower 32-bit */ \
        AUW(EMPTY,    VAL(IT), TMxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x780007C6 | MSM(TMxx,    TMxx,    0x00))                     \
        AUW(SIB(MD),  VAL(IS), TMxx+32, MOD(MD), VAL(DD), C1(DD), G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MD), VAL(DD), B1(DD), Q1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))

#define andzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define andzx_rr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))

#define andzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))

#define andzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define andzx_mr(MG, DG, RS)                                                \
        andzx_st(W(RS), W(MG), W(DG))


#define andzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))

#define andzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define andzxZrr(RG, RS)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RS)))

#define andzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define andzxZmr(MG, DG, RS)                                                \
        andzxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annzx_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzx_ri(W(RG), W(IS))

#define annzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000038))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define annzx_rr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RS), REG(RG)))

#define annzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000078 | MSM(REG(RG), TMxx,    REG(RG)))

#define annzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000078 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define annzx_mr(MG, DG, RS)                                                \
        annzx_st(W(RS), W(MG), W(DG))


#define annzxZri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        andzxZri(W(RG), W(IS))

#define annzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x70000000) | (M(TP2(IS) != 0) & 0x7C000039))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define annzxZrr(RG, RS)                                                    \
        EMITW(0x7C000079 | MSM(REG(RG), REG(RS), REG(RG)))

#define annzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000079 | MSM(REG(RG), TMxx,    REG(RG)))

#define annzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000079 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define annzxZmr(MG, DG, RS)                                                \
        annzxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))

#define orrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define orrzx_rr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))

#define orrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define orrzx_mr(MG, DG, RS)                                                \
        orrzx_st(W(RS), W(MG), W(DG))


#define orrzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), RXG(RG), EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x28200000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))  \
        EMITW(0x28200000 | TMxx << 16)                 /* <- set flags (Z) */

#define orrzxZrr(RG, RS)                                                    \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), TMxx))

#define orrzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000379 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define orrzxZmr(MG, DG, RS)                                                \
        orrzxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornzx_ri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzx_ri(W(RG), W(IS))

#define ornzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define ornzx_rr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000338 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000338 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define ornzx_mr(MG, DG, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DG))


#define ornzxZri(RG, IS)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzxZri(W(RG), W(IS))

#define ornzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        AUW(EMPTY,    VAL(IS), TMxx+32, EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))  \
        EMITW(0x28200000 | TMxx << 16)                 /* <- set flags (Z) */

#define ornzxZrr(RG, RS)                                                    \
        EMITW(0x7C000339 | MSM(REG(RG), REG(RS), REG(RG)))

#define ornzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000339 | MSM(REG(RG), TMxx,    REG(RG)))

#define ornzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000339 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define ornzxZmr(MG, DG, RS)                                                \
        ornzxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))

#define xorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define xorzx_rr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))

#define xorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define xorzx_mr(MG, DG, RS)                                                \
        xorzx_st(W(RS), W(MG), W(DG))


#define xorzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x28200000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G2(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x68000000) | (M(TP2(IS) != 0) & 0x7C000278))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))  \
        EMITW(0x28200000 | TMxx << 16)                 /* <- set flags (Z) */

#define xorzxZrr(RG, RS)                                                    \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), TMxx))

#define xorzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000279 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define xorzxZmr(MG, DG, RS)                                                \
        xorzxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RG), REG(RG), REG(RG)))

#define notzx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C0000F8 | MSM(TWxx,    TWxx,    TWxx))                     \
        EMITW(0x00000000 | MDM(TWxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negzx_rx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))

#define negzx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))


#define negzxZrx(RG)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RG), 0x00,    REG(RG)))

#define negzxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))

#define addzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x38000000) | (M(TP1(IS) != 0) & 0x7C000214))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define addzx_rr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define addzx_mr(MG, DG, RS)                                                \
        addzx_st(W(RS), W(MG), W(DG))


#define addzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))

#define addzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IS), T1(IS), M1(IS)) | \
        (M(TP1(IS) == 0) & 0x34000000) | (M(TP1(IS) != 0) & 0x7C000215))    \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define addzxZrr(RG, RS)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define addzxZmr(MG, DG, RS)                                                \
        addzxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))

#define subzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x38000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define subzx_rr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define subzx_mr(MG, DG, RS)                                                \
        subzx_st(W(RS), W(MG), W(DG))


#define subzxZri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))

#define subzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  VAL(IS), TIxx,    MOD(MG), VAL(DG), C1(DG), G1(IS))   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IS), EMPTY1) | \
        (M(TP1(IS) == 0) & (0x34000000 | (0xFFFF & -VAL(IS)))) |            \
        (M(TP1(IS) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define subzxZrr(RG, RS)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define subzxZmr(MG, DG, RS)                                                \
        subzxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000036 | MSM(REG(RG), REG(RG), Tecx))

#define shlzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000036 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shlzx_ri(RG, IS)                                                    \
        EMITW(0x78000004 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
              ((63-VAL(IS)) & 0x1F) << 6 | ((63-VAL(IS)) & 0x20) |          \
                                                (VAL(IS) & 0x20) >> 4)

#define shlzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x78000004 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
              ((63-VAL(IS)) & 0x1F) << 6 | ((63-VAL(IS)) & 0x20) |          \
                                                (VAL(IS) & 0x20) >> 4)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000036 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000036 | MSM(REG(RG), REG(RG), TMxx))

#define shlzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000036 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shlzx_mr(MG, DG, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DG))


#define shlzxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000037 | MSM(REG(RG), REG(RG), Tecx))

#define shlzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000037 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shlzxZri(RG, IS)                                                    \
        EMITW(0x78000005 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
              ((63-VAL(IS)) & 0x1F) << 6 | ((63-VAL(IS)) & 0x20) |          \
                                                (VAL(IS) & 0x20) >> 4)

#define shlzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x78000005 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
              ((63-VAL(IS)) & 0x1F) << 6 | ((63-VAL(IS)) & 0x20) |          \
                                                (VAL(IS) & 0x20) >> 4)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shlzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000037 | MSM(REG(RG), REG(RG), REG(RS)))

#define shlzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000037 | MSM(REG(RG), REG(RG), TMxx))

#define shlzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000037 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shlzxZmr(MG, DG, RS)                                                \
        shlzxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000436 | MSM(REG(RG), REG(RG), Tecx))

#define shrzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000436 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzx_ri(RG, IS)                                                    \
        EMITW(0x78000000 | MSM(REG(RG), REG(RG), (64-VAL(IS)) & 0x1F) |     \
                             (VAL(IS) & 0x1F) << 6 | (VAL(IS) & 0x20) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4)

#define shrzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x78000000 | MSM(TMxx,    TMxx,    (64-VAL(IS)) & 0x1F) |     \
                             (VAL(IS) & 0x1F) << 6 | (VAL(IS) & 0x20) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000436 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000436 | MSM(REG(RG), REG(RG), TMxx))

#define shrzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000436 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzx_mr(MG, DG, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DG))


#define shrzxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000437 | MSM(REG(RG), REG(RG), Tecx))

#define shrzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000437 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzxZri(RG, IS)                                                    \
        EMITW(0x78000001 | MSM(REG(RG), REG(RG), (64-VAL(IS)) & 0x1F) |     \
                             (VAL(IS) & 0x1F) << 6 | (VAL(IS) & 0x20) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4)

#define shrzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x78000001 | MSM(TMxx,    TMxx,    (64-VAL(IS)) & 0x1F) |     \
                             (VAL(IS) & 0x1F) << 6 | (VAL(IS) & 0x20) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000437 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000437 | MSM(REG(RG), REG(RG), TMxx))

#define shrzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000437 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzxZmr(MG, DG, RS)                                                \
        shrzxZst(W(RS), W(MG), W(DG))


#define shrzn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000634 | MSM(REG(RG), REG(RG), Tecx))

#define shrzn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000634 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzn_ri(RG, IS)                                                    \
        EMITW(0x7C000674 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                                (VAL(IS) & 0x20) >> 4)

#define shrzn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000674 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                                (VAL(IS) & 0x20) >> 4)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000634 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrzn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000634 | MSM(REG(RG), REG(RG), TMxx))

#define shrzn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000634 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrzn_mr(MG, DG, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DG))


#define shrznZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x7C000635 | MSM(REG(RG), REG(RG), Tecx))

#define shrznZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000635 | MSM(TMxx,    TMxx,    Tecx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrznZri(RG, IS)                                                    \
        EMITW(0x7C000675 | MSM(REG(RG), REG(RG), VAL(IS) & 0x1F) |          \
                                                (VAL(IS) & 0x20) >> 4)

#define shrznZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000675 | MSM(TMxx,    TMxx,    VAL(IS) & 0x1F) |          \
                                                (VAL(IS) & 0x20) >> 4)      \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrznZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000635 | MSM(REG(RG), REG(RG), REG(RS)))

#define shrznZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000635 | MSM(REG(RG), REG(RG), TMxx))

#define shrznZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x7C000635 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define shrznZmr(MG, DG, RS)                                                \
        shrznZst(W(RS), W(MG), W(DG))

/* ror (G = G >> S | G << 64 - S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define rorzx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x78000010 | MSM(REG(RG), REG(RG), TIxx))

#define rorzx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x78000010 | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define rorzx_ri(RG, IS)                                                    \
        EMITW(0x78000000 | MSM(REG(RG), REG(RG), (64-VAL(IS)) & 0x1F) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4)

#define rorzx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x78000000 | MSM(TMxx,    TMxx,    (64-VAL(IS)) & 0x1F) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define rorzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x78000010 | MSM(REG(RG), REG(RG), TIxx))

#define rorzx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    TMxx))                     \
        EMITW(0x78000010 | MSM(REG(RG), REG(RG), TIxx))

#define rorzx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x78000010 | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define rorzx_mr(MG, DG, RS)                                                \
        rorzx_st(W(RS), W(MG), W(DG))


#define rorzxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x78000011 | MSM(REG(RG), REG(RG), TIxx))

#define rorzxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    Tecx))                     \
        EMITW(0x78000011 | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define rorzxZri(RG, IS)                                                    \
        EMITW(0x78000001 | MSM(REG(RG), REG(RG), (64-VAL(IS)) & 0x1F) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4)

#define rorzxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x78000001 | MSM(TMxx,    TMxx,    (64-VAL(IS)) & 0x1F) |     \
                                                ((64-VAL(IS)) & 0x20) >> 4) \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define rorzxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x78000011 | MSM(REG(RG), REG(RG), TIxx))

#define rorzxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    TMxx))                     \
        EMITW(0x78000011 | MSM(REG(RG), REG(RG), TIxx))

#define rorzxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DG), C1(DG), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), F1(DG)))  \
        EMITW(0x20000040 | MRM(TIxx,    0x00,    REG(RS)))                  \
        EMITW(0x78000011 | MSM(TMxx,    TMxx,    TIxx))                     \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MG), VAL(DG), B1(DG), Q1(DG)))

#define rorzxZmr(MG, DG, RS)                                                \
        rorzxZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulzx_ri(RG, IS)                                                    \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), TIxx))

#define mulzx_rr(RG, RS)                                                    \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulzx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), TMxx))


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C000012 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    REG(RS)))

#define mulzx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000012 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    TMxx))


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C000092 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    REG(RS)))

#define mulzn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000092 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    TMxx))


#define mulzp_xr(RS)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulzp_xm(MS, DS) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_ld(Reax, W(MS), W(DS))  /* must not exceed operands size */

/* div (G = G / S)
 * set-flags: undefined */

#define divzx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), TIxx))

#define divzx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), REG(RS)))

#define divzx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), TMxx))


#define divzn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), TIxx))

#define divzn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), REG(RS)))

#define divzn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), TMxx))


#define prezx_xx()          /* to be placed immediately prior divzx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prezn_xx()          /* to be placed immediately prior divzn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divzx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000392 | MTM(Teax,    Teax,    REG(RS)))

#define divzx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000392 | MTM(Teax,    Teax,    TMxx))


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D2 | MTM(Teax,    Teax,    REG(RS)))

#define divzn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C0003D2 | MTM(Teax,    Teax,    TMxx))


#define divzp_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xr(W(RS))              /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

#define divzp_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        divzn_xm(W(MS), W(DS))       /* destroys Redx, Xmm0 (in ARMv7) */   \
                                     /* 24-bit int (fp32 div in ARMv7) */

/* rem (G = G % S)
 * set-flags: undefined */

#if RT_BASE_COMPAT_REM == 0

#define remzx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remzn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ri(W(RG), W(IS))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ld(W(RG), W(MS), W(DS))                                       \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remzx_xm(MS, DS)    /* to be placed immediately after divzx_xm */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remzn_xm(MS, DS)    /* to be placed immediately after divzn_xm */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#else /* RT_BASE_COMPAT_REM != 0 */

#define remzx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000212 | MTM(REG(RG), REG(RG), TIxx))

#define remzx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000212 | MTM(REG(RG), REG(RG), REG(RS)))

#define remzx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000212 | MTM(REG(RG), REG(RG), TMxx))


#define remzn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IS))   \
        EMITW(0x7C000612 | MTM(REG(RG), REG(RG), TIxx))

#define remzn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        EMITW(0x7C000612 | MTM(REG(RG), REG(RG), REG(RS)))

#define remzn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000612 | MTM(REG(RG), REG(RG), TMxx))


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
        EMITW(0x7C000212 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remzx_xm(MS, DS)    /* to be placed immediately after divzx_xm */   \
        EMITW(0x7C000212 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
        EMITW(0x7C000612 | MTM(Tedx,    Tedx,    REG(RS)))/* Redx<-rem */

#define remzn_xm(MS, DS)    /* to be placed immediately after divzn_xm */   \
        EMITW(0x7C000612 | MTM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#endif /* RT_BASE_COMPAT_REM != 0 */

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
        CXI(cc, MOD(RS), REG(RS), W(IT), lb)

#define cmjzx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        CXI(cc, %%r24,   TMxx,    W(IT), lb)

#define cmjzx_rr(RS, RT, cc, lb)                                            \
        CXR(cc, MOD(RS), MOD(RT), lb)

#define cmjzx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        CXR(cc, MOD(RS), %%r24,   lb)

#define cmjzx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        CXR(cc, %%r24,   MOD(RT), lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpwx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x7C0007B4 | MSM(TLxx,    REG(RS), 0x00))                     \
        EMITW(0x7C0007B4 | MSM(TRxx,    TRxx,    0x00))

#define cmpwx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PW(DS)))  \
        EMITW(0x7C0007B4 | MSM(TRxx,    TRxx,    0x00))

#define cmpwx_rr(RS, RT)                                                    \
        EMITW(0x7C0007B4 | MSM(TRxx,    REG(RT), 0x00))                     \
        EMITW(0x7C0007B4 | MSM(TLxx,    REG(RS), 0x00))

#define cmpwx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), PW(DT)))  \
        EMITW(0x7C0007B4 | MSM(TLxx,    REG(RS), 0x00))

#define cmpwx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), PW(DS)))  \
        EMITW(0x7C0007B4 | MSM(TRxx,    REG(RT), 0x00))


#define cmpzx_ri(RS, IT)                                                    \
        AUW(EMPTY,    VAL(IT), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IT))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpzx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  VAL(IT), TRxx,    MOD(MS), VAL(DS), C1(DS), G3(IT))   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))

#define cmpzx_rr(RS, RT)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpzx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DT), C1(DT), EMPTY2)   \
        EMITW(0x00000000 | MDM(TRxx,    MOD(MT), VAL(DT), B1(DT), F1(DT)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpzx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TLxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

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

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        EMITW(0x7C0003A6 | MRM(REG(RS), 0x00,    0x09)) /* ctr <- reg */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#define jmpxx_xm(MS, DS)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DS), C1(DS), EMPTY2)   \
        EMITW(0x00000000 | MDM(TMxx,    MOD(MS), VAL(DS), B1(DS), F1(DS)))  \
        EMITW(0x7C0003A6 | MRM(TMxx,    0x00,    0x09)) /* ctr <- r24 */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#define jmpxx_lb(lb)              /* label-targeted unconditional jump */   \
        ASM_BEG ASM_OP1(b, lb) ASM_END

#define jezxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define jnzxx_lb(lb)               /* setting-flags-arithmetic -> jump */   \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define jeqxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define jnexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define jltxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define jlexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define jgtxx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define jgexx_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpld, %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define jltxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define jlexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define jgtxn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define jgexn_lb(lb)                                /* compare -> jump */   \
        ASM_BEG ASM_OP2(cmpd,  %%r24, %%r25) ASM_END                        \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define LBL(lb)                                          /* code label */   \
        ASM_BEG ASM_OP0(lb:) ASM_END

/************************* register-size instructions *************************/

/* stack (push stack = S, D = pop stack)
 * set-flags: no (sequence cmp/stack_la/jmp is not allowed on MIPS & POWER)
 * adjust stack pointer with 8-byte (64-bit) steps on all current targets */

#define stack_st(RS)                                                        \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0x08 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(REG(RS), SPxx,    0x00))

#define stack_ld(RD)                                                        \
        EMITW(0xE8000000 | MTM(REG(RD), SPxx,    0x00))                     \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0x08 & 0xFFFF))

#define stack_sa()  /* save all, [Reax - RegE] + 12 temps, 26 regs total */ \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (-0xD0 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teax,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tecx,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tedx,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tebx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tebp,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tesi,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Tedi,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teg8,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(Teg9,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegA,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegB,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegC,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegD,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TegE,    SPxx,    0x00) | (+0x68 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TMxx,    SPxx,    0x00) | (+0x70 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TIxx,    SPxx,    0x00) | (+0x78 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TDxx,    SPxx,    0x00) | (+0x80 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TPxx,    SPxx,    0x00) | (+0x88 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TCxx,    SPxx,    0x00) | (+0x90 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TVxx,    SPxx,    0x00) | (+0x98 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(T0xx,    SPxx,    0x00) | (+0xA0 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(T1xx,    SPxx,    0x00) | (+0xA8 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(T2xx,    SPxx,    0x00) | (+0xB0 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(T3xx,    SPxx,    0x00) | (+0xB8 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TZxx,    SPxx,    0x00) | (+0xC0 & 0xFFFF))  \
        EMITW(0xF8000000 | MTM(TWxx,    SPxx,    0x00) | (+0xC8 & 0xFFFF))

#define stack_la()  /* load all, 12 temps + [RegE - Reax], 26 regs total */ \
        EMITW(0xE8000000 | MTM(TWxx,    SPxx,    0x00) | (+0xC8 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TZxx,    SPxx,    0x00) | (+0xC0 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(T3xx,    SPxx,    0x00) | (+0xB8 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(T2xx,    SPxx,    0x00) | (+0xB0 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(T1xx,    SPxx,    0x00) | (+0xA8 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(T0xx,    SPxx,    0x00) | (+0xA0 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TVxx,    SPxx,    0x00) | (+0x98 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TCxx,    SPxx,    0x00) | (+0x90 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TPxx,    SPxx,    0x00) | (+0x88 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TDxx,    SPxx,    0x00) | (+0x80 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TIxx,    SPxx,    0x00) | (+0x78 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TMxx,    SPxx,    0x00) | (+0x70 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegE,    SPxx,    0x00) | (+0x68 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegD,    SPxx,    0x00) | (+0x60 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegC,    SPxx,    0x00) | (+0x58 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegB,    SPxx,    0x00) | (+0x50 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(TegA,    SPxx,    0x00) | (+0x48 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teg9,    SPxx,    0x00) | (+0x40 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teg8,    SPxx,    0x00) | (+0x38 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tedi,    SPxx,    0x00) | (+0x30 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tesi,    SPxx,    0x00) | (+0x28 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tebp,    SPxx,    0x00) | (+0x20 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tebx,    SPxx,    0x00) | (+0x18 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tedx,    SPxx,    0x00) | (+0x10 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Tecx,    SPxx,    0x00) | (+0x08 & 0xFFFF))  \
        EMITW(0xE8000000 | MTM(Teax,    SPxx,    0x00) | (+0x00 & 0xFFFF))  \
        EMITW(0x38000000 | MTM(SPxx,    SPxx,    0x00) | (+0xD0 & 0xFFFF))

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/* internal definitions for combined-compare-jump (cmj) */

#define IXJ0(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28200000) | (M(TP2(IS) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define IXJ1(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28200000) | (M(TP2(IS) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define IXJ2(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28200000) | (M(TP2(IS) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IXJ3(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28200000) | (M(TP2(IS) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IXJ4(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28200000) | (M(TP2(IS) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IXJ5(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T2(IS), M2(IS)) | \
        (M(TP2(IS) == 0) & 0x28200000) | (M(TP2(IS) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define IXJ6(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C200000) | (M(TP1(IS) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IXJ7(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C200000) | (M(TP1(IS) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IXJ8(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C200000) | (M(TP1(IS) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IXJ9(r1, p1, IS, lb)                                                \
        AUW(EMPTY,    VAL(IS), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IS))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IS), T3(IS), M3(IS)) | \
        (M(TP1(IS) == 0) & 0x2C200000) | (M(TP1(IS) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CXI(cc, r1, p1, IS, lb)                                             \
        IX##cc(r1, p1, W(IS), lb)


#define RXJ0(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define RXJ1(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define RXJ2(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RXJ3(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RXJ4(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RXJ5(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpld, r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define RXJ6(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define RXJ7(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define RXJ8(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define RXJ9(r1, r2, lb)                                                    \
        ASM_BEG ASM_OP2(cmpd,  r1, r2) ASM_END                              \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CXR(cc, r1, r2, lb)                                                 \
        RX##cc(r1, r2, lb)

#endif /* (defined RT_P64) */

#endif /* RT_RTARCH_P64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/******************************************************************************/
/* Copyright (c) 2013-2016 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_P64_H
#define RT_RTARCH_P64_H

#define RT_BASE_REGS        16

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_p64.h: Implementation of Power 64-bit BASE instructions.
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

#include "rtarch_p32.h"

/******************************************************************************/
/**********************************   P64   ***********************************/
/******************************************************************************/

/* mov
 * set-flags: no */

#define movzx_ri(RD, IM)                                                    \
        AUW(EMPTY,    VAL(IM), REG(RD), EMPTY,   EMPTY,   EMPTY2, G3(IM))

#define movzx_mi(MD, DP, IM)                                                \
        AUW(SIB(MD),  VAL(IM), TDxx,    MOD(MD), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xF8000000 | MDM(TDxx,    MOD(MD), VAL(DP), B1(DP), P1(DP)))

#define movzx_rr(RD, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RD), REG(RS), REG(RS)))

#define movzx_ld(RD, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(REG(RD), MOD(MS), VAL(DP), B1(DP), P1(DP)))

#define movzx_st(RS, MD, DP)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    MOD(MD), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xF8000000 | MDM(REG(RS), MOD(MD), VAL(DP), B1(DP), P1(DP)))

/* and
 * set-flags: undefined (*x), yes (*z) */

#define andzx_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        /* if true ^ equals to -1 (not 1) */

#define andzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define andzx_rr(RG, RS)                                                    \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), REG(RS)))

#define andzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(REG(RG), REG(RG), TMxx))

#define andzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000038 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))


#define andzz_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000039))    \
        /* if true ^ equals to -1 (not 1) */

#define andzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000039))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define andzz_rr(RG, RS)                                                    \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), REG(RS)))

#define andzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000039 | MSM(REG(RG), REG(RG), TMxx))

#define andzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000039 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

/* ann (G = ~G & S)
 * set-flags: undefined (*x), yes (*z) */

#define annzx_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        andzx_ri(W(RG), W(IM))

#define annzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000038))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzx_rr(RG, RS)                                                    \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RG), REG(RS)))

#define annzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000078 | MSM(REG(RG), REG(RG), TMxx))

#define annzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000078 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzx_mr(MG, DP, RS)                                                \
        annzx_st(W(RS), W(MG), W(DP))


#define annzz_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        andzz_ri(W(RG), W(IM))

#define annzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x70000000) | (+(TP2(IM) != 0) & 0x7C000039))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzz_rr(RG, RS)                                                    \
        EMITW(0x7C000079 | MSM(REG(RG), REG(RG), REG(RS)))

#define annzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000079 | MSM(REG(RG), REG(RG), TMxx))

#define annzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000079 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define annzz_mr(MG, DP, RS)                                                \
        annzz_st(W(RS), W(MG), W(DP))

/* orr
 * set-flags: undefined (*x), yes (*z) */

#define orrzx_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        /* if true ^ equals to -1 (not 1) */

#define orrzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define orrzx_rr(RG, RS)                                                    \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(REG(RG), REG(RG), TMxx))

#define orrzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))


#define orrzz_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28200000 | REG(RG) << 16)              /* <- set flags (Z) */

#define orrzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28200000 | TMxx << 16)                 /* <- set flags (Z) */

#define orrzz_rr(RG, RS)                                                    \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), REG(RS)))

#define orrzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000379 | MSM(REG(RG), REG(RG), TMxx))

#define orrzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000379 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

/* orn (G = ~G | S)
 * set-flags: undefined (*x), yes (*z) */

#define ornzx_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzx_ri(W(RG), W(IM))

#define ornzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define ornzx_rr(RG, RS)                                                    \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RG), REG(RS)))

#define ornzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000338 | MSM(REG(RG), REG(RG), TMxx))

#define ornzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000338 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define ornzx_mr(MG, DP, RS)                                                \
        ornzx_st(W(RS), W(MG), W(DP))


#define ornzz_ri(RG, IM)                                                    \
        notzx_rx(W(RG))                                                     \
        orrzz_ri(W(RG), W(IM))

#define ornzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TMxx,    TMxx,    TMxx))                     \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x60000000) | (+(TP2(IM) != 0) & 0x7C000378))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28200000 | TMxx << 16)                 /* <- set flags (Z) */

#define ornzz_rr(RG, RS)                                                    \
        EMITW(0x7C000339 | MSM(REG(RG), REG(RG), REG(RS)))

#define ornzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000339 | MSM(REG(RG), REG(RG), TMxx))

#define ornzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000339 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define ornzz_mr(MG, DP, RS)                                                \
        ornzz_st(W(RS), W(MG), W(DP))

/* xor
 * set-flags: undefined (*x), yes (*z) */

#define xorzx_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        /* if true ^ equals to -1 (not 1) */

#define xorzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define xorzx_rr(RG, RS)                                                    \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(REG(RG), REG(RG), TMxx))

#define xorzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000278 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))


#define xorzz_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        /* if true ^ equals to -1 (not 1) */                                \
        EMITW(0x28200000 | REG(RG) << 16)              /* <- set flags (Z) */

#define xorzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G2(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x68000000) | (+(TP2(IM) != 0) & 0x7C000278))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x28200000 | TMxx << 16)                 /* <- set flags (Z) */

#define xorzz_rr(RG, RS)                                                    \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), REG(RS)))

#define xorzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000279 | MSM(REG(RG), REG(RG), TMxx))

#define xorzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000279 | MSM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

/* not
 * set-flags: no */

#define notzx_rx(RG)                                                        \
        EMITW(0x7C0000F8 | MSM(REG(RG), REG(RG), REG(RG)))

#define notzx_mx(MG, DP)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TDxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000F8 | MSM(TDxx,    TDxx,    TDxx))                     \
        EMITW(0xF8000000 | MDM(TDxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

/* neg
 * set-flags: undefined (*x), yes (*z) */

#define negzx_rx(RG)                                                        \
        EMITW(0x7C0000D0 | MRM(REG(RG), 0x00,    REG(RG)))

#define negzx_mx(MG, DP)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D0 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))


#define negzz_rx(RG)                                                        \
        EMITW(0x7C0000D1 | MRM(REG(RG), 0x00,    REG(RG)))

#define negzz_mx(MG, DP)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0000D1 | MRM(TMxx,    0x00,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

/* add
 * set-flags: undefined (*x), yes (*z) */

#define addzx_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        /* if true ^ equals to -1 (not 1) */

#define addzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x38000000) | (+(TP1(IM) != 0) & 0x7C000214))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define addzx_rr(RG, RS)                                                    \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(REG(RG), REG(RG), TMxx))

#define addzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000214 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))


#define addzz_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x34000000) | (+(TP1(IM) != 0) & 0x7C000215))    \
        /* if true ^ equals to -1 (not 1) */

#define addzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    VAL(IM), T1(IM), M1(IM)) | \
        (+(TP1(IM) == 0) & 0x34000000) | (+(TP1(IM) != 0) & 0x7C000215))    \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define addzz_rr(RG, RS)                                                    \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), REG(RS)))

#define addzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000215 | MRM(REG(RG), REG(RG), TMxx))

#define addzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000215 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

/* sub
 * set-flags: undefined (*x), yes (*z) */

#define subzx_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x38000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000050 | TIxx << 16)))                      \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define subzx_rr(RG, RS)                                                    \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(REG(RG), REG(RG), TMxx))

#define subzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000050 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define subzx_mr(MG, DP, RS)                                                \
        subzx_st(W(RS), W(MG), W(DP))


#define subzz_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(REG(RG), REG(RG), 0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x34000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000051 | TIxx << 16)))                      \
        /* if true ^ equals to -1 (not 1) */

#define subzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  VAL(IM), TIxx,    MOD(MG), VAL(DP), C1(DP), G1(IM))   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x00000000 | MIM(TMxx,    TMxx,    0x00,    T1(IM), EMPTY1) | \
        (+(TP1(IM) == 0) & (0x34000000 | (0xFFFF & -VAL(IM)))) |            \
        (+(TP1(IM) != 0) & (0x7C000051 | TIxx << 16)))                      \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define subzz_rr(RG, RS)                                                    \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), REG(RS)))

#define subzz_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000051 | MRM(REG(RG), REG(RG), TMxx))

#define subzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000051 | MRM(TMxx,    TMxx,    REG(RS)))                  \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define subzz_mr(MG, DP, RS)                                                \
        subzz_st(W(RS), W(MG), W(DP))

/* shl
 * set-flags: undefined (*x), yes (*z) */

#define shlzx_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x7C000036 | MSM(REG(RG), Tecx,    REG(RG)))

#define shlzx_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000036 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shlzx_ri(RG, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0x7C000036 | MSM(REG(RG), TIxx,    REG(RG)))

#define shlzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000036 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shlzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000036 | MSM(REG(RG), REG(RS), REG(RG)))

#define shlzx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000036 | MSM(REG(RG), TMxx,    REG(RG)))

#define shlzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000036 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shlzx_mr(MG, DP, RS)                                                \
        shlzx_st(W(RS), W(MG), W(DP))


#define shlzz_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x7C000037 | MSM(REG(RG), Tecx,    REG(RG)))

#define shlzz_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000037 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shlzz_ri(RG, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0x7C000037 | MSM(REG(RG), TIxx,    REG(RG)))

#define shlzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000037 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shlzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000037 | MSM(REG(RG), REG(RS), REG(RG)))

#define shlzz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000037 | MSM(REG(RG), TMxx,    REG(RG)))

#define shlzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000037 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shlzz_mr(MG, DP, RS)                                                \
        shlzz_st(W(RS), W(MG), W(DP))

/* shr
 * set-flags: undefined (*x), yes (*z) */

#define shrzx_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x7C000436 | MSM(REG(RG), Tecx,    REG(RG)))

#define shrzx_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000436 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzx_ri(RG, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0x7C000436 | MSM(REG(RG), TIxx,    REG(RG)))

#define shrzx_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000436 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000436 | MSM(REG(RG), REG(RS), REG(RG)))

#define shrzx_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000436 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrzx_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000436 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzx_mr(MG, DP, RS)                                                \
        shrzx_st(W(RS), W(MG), W(DP))


#define shrzz_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x7C000437 | MSM(REG(RG), Tecx,    REG(RG)))

#define shrzz_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000437 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzz_ri(RG, IM)                                                    \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0x7C000437 | MSM(REG(RG), TIxx,    REG(RG)))

#define shrzz_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0x60000000 | TIxx << 16 | (0x3F & VAL(IM)))                   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000437 | MSM(TMxx,    TIxx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzz_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000437 | MSM(REG(RG), REG(RS), REG(RG)))

#define shrzz_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000437 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrzz_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000437 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzz_mr(MG, DP, RS)                                                \
        shrzz_st(W(RS), W(MG), W(DP))


#define shrzn_rx(RG)                     /* reads Recx for shift value */   \
        EMITW(0x7C000634 | MSM(REG(RG), Tecx,    REG(RG)))

#define shrzn_mx(MG, DP)                 /* reads Recx for shift value */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000634 | MSM(TMxx,    Tecx,    TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzn_ri(RG, IM)                                                    \
        EMITW(0x7C000674 | MSM(REG(RG), (0x1F & VAL(IM)), REG(RG)) |        \
                                        (0x02 & VAL(IM) >> 4))

#define shrzn_mi(MG, DP, IM)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000674 | MSM(TMxx,    (0x1F & VAL(IM)), TMxx) |           \
                                        (0x02 & VAL(IM) >> 4))              \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(0x7C000634 | MSM(REG(RG), REG(RS), REG(RG)))

#define shrzn_ld(RG, MS, DP)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000634 | MSM(REG(RG), TMxx,    REG(RG)))

#define shrzn_st(RS, MG, DP)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    MOD(MG), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000634 | MSM(TMxx,    REG(RS), TMxx))                     \
        EMITW(0xF8000000 | MDM(TMxx,    MOD(MG), VAL(DP), B1(DP), P1(DP)))

#define shrzn_mr(MG, DP, RS)                                                \
        shrzn_st(W(RS), W(MG), W(DP))

/* mul
 * set-flags: undefined */

#define mulzx_ri(RG, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), TIxx))

#define mulzx_rr(RG, RS)                                                    \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), REG(RS)))

#define mulzx_ld(RG, MS, DP)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0001D2 | MRM(REG(RG), REG(RG), TMxx))


#define mulzx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(0x7C000012 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    REG(RS)))

#define mulzx_xm(MS, DP) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000012 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    TMxx))


#define mulzn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(0x7C000092 | MRM(Tedx,    Teax,    REG(RS)))                  \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    REG(RS)))

#define mulzn_xm(MS, DP) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000092 | MRM(Tedx,    Teax,    TMxx))                     \
        EMITW(0x7C0001D2 | MRM(Teax,    Teax,    TMxx))


#define mulzp_xr(RS)     /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_rr(Reax, W(RS)) /* product must not exceed operands size */

#define mulzp_xm(MS, DP) /* Reax is in/out, prepares Redx for divzn_x* */   \
        mulzx_ld(Reax, W(MS), W(DP))  /* must not exceed operands size */

/* div
 * set-flags: undefined */

#define divzx_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), TIxx))

#define divzx_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), REG(RS)))

#define divzx_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000392 | MTM(REG(RG), REG(RG), TMxx))


#define divzn_ri(RG, IM)       /* Reax cannot be used as first operand */   \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), TIxx))

#define divzn_rr(RG, RS)                 /* RG, RS no Reax, RS no Redx */   \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), REG(RS)))

#define divzn_ld(RG, MS, DP)   /* Reax cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D2 | MTM(REG(RG), REG(RG), TMxx))


#define prezx_xx()          /* to be placed immediately prior divzx_x* */   \
                                     /* to prepare Redx for int-divide */

#define prezn_xx()          /* to be placed immediately prior divzn_x* */   \
                                     /* to prepare Redx for int-divide */


#define divzx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(0x7C000392 | MTM(Teax,    Teax,    REG(RS)))

#define divzx_xm(MS, DP) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000392 | MTM(Teax,    Teax,    TMxx))


#define divzn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(0x7C0003D2 | MTM(Teax,    Teax,    REG(RS)))

#define divzn_xm(MS, DP) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0003D2 | MTM(Teax,    Teax,    TMxx))


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
        movzx_rr(Redx, W(RG))                                               \
        divzx_ri(W(RG), W(IM))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzx_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzx_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzx_ld(W(RG), W(MS), W(DP))                                       \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remzn_ri(RG, IM)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ri(W(RG), W(IM))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TIxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzn_rr(RG, RS)                 /* RG, RS no Redx, RS no Reax */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_rr(W(RG), W(RS))                                              \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), REG(RS)))                  \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)

#define remzn_ld(RG, MS, DP)   /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        movzx_rr(Redx, W(RG))                                               \
        divzn_ld(W(RG), W(MS), W(DP))                                       \
        EMITW(0x7C0001D2 | MRM(TMxx,    REG(RG), TMxx))                     \
        EMITW(0x7C000050 | MRM(REG(RG), Tedx,    TMxx))                     \
        stack_ld(Redx)


#define remzx_xx()          /* to be placed immediately prior divzx_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzx_xr(RS)        /* to be placed immediately after divzx_xr */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remzx_xm(MS, DP)    /* to be placed immediately after divzx_xm */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */


#define remzn_xx()          /* to be placed immediately prior divzn_x* */   \
        movzx_rr(Redx, Reax)         /* to prepare for rem calculation */

#define remzn_xr(RS)        /* to be placed immediately after divzn_xr */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    REG(RS)))                  \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

#define remzn_xm(MS, DP)    /* to be placed immediately after divzn_xm */   \
        EMITW(0x7C0001D2 | MRM(TMxx,    Teax,    TMxx))                     \
        EMITW(0x7C000050 | MRM(Tedx,    Tedx,    TMxx))   /* Redx<-rem */

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
        CXI(cc, MOD(RS), REG(RS), W(IM), lb)

#define cmjzx_mi(MS, DP, IM, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        CXI(cc, %%r24,   TMxx,    W(IM), lb)

#define cmjzx_rr(RS, RT, cc, lb)                                            \
        CXR(cc, MOD(RS), MOD(RT), lb)

#define cmjzx_rm(RS, MT, DP, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MT), VAL(DP), B1(DP), P1(DP)))  \
        CXR(cc, MOD(RS), %%r24,   lb)

#define cmjzx_mr(MS, DP, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        CXR(cc, %%r24,   MOD(RT), lb)

/* internal definitions for combined-compare-jump (cmj) */

#define IXJ0(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(beq,   lb) ASM_END

#define IXJ1(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bne,   lb) ASM_END

#define IXJ2(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IXJ3(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IXJ4(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IXJ5(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G2(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T2(IM), M2(IM)) | \
        (+(TP2(IM) == 0) & 0x28200000) | (+(TP2(IM) != 0) & 0x7C200040))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define IXJ6(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(blt,   lb) ASM_END

#define IXJ7(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(ble,   lb) ASM_END

#define IXJ8(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(bgt,   lb) ASM_END

#define IXJ9(r1, p1, IM, lb)                                                \
        AUW(EMPTY,    VAL(IM), TIxx,    EMPTY,   EMPTY,   EMPTY2, G1(IM))   \
        EMITW(0x00000000 | MIM(p1,      0x00,    VAL(IM), T3(IM), M3(IM)) | \
        (+(TP2(IM) == 0) & 0x2C200000) | (+(TP2(IM) != 0) & 0x7C200000))    \
        ASM_BEG ASM_OP1(bge,   lb) ASM_END

#define CXI(cc, r1, p1, IM, lb)                                             \
        IX##cc(r1, p1, W(IM), lb)


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

/* cmp
 * set-flags: yes */

#define cmpwx_ri(RS, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C0007B4 | MSM(TLxx,    0x00,    REG(RS)))                  \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    TRxx))

#define cmpwx_mi(MS, DP, IM)                                                \
        AUW(SIB(MS),  VAL(IM), TRxx,    MOD(MS), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xE8000002 | MDM(TLxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    TRxx))

#define cmpwx_rr(RS, RT)                                                    \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    REG(RT)))                  \
        EMITW(0x7C0007B4 | MSM(TLxx,    0x00,    REG(RS)))

#define cmpwx_rm(RS, MT, DP)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000002 | MDM(TRxx,    MOD(MT), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0007B4 | MSM(TLxx,    0x00,    REG(RS)))

#define cmpwx_mr(MS, DP, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000002 | MDM(TLxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C0007B4 | MSM(TRxx,    0x00,    REG(RT)))


#define cmpzx_ri(RS, IM)                                                    \
        AUW(EMPTY,    VAL(IM), TRxx,    EMPTY,   EMPTY,   EMPTY2, G3(IM))   \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpzx_mi(MS, DP, IM)                                                \
        AUW(SIB(MS),  VAL(IM), TRxx,    MOD(MS), VAL(DP), C1(DP), G3(IM))   \
        EMITW(0xE8000000 | MDM(TLxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))

#define cmpzx_rr(RS, RT)                                                    \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))                  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpzx_rm(RS, MT, DP)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    MOD(MT), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TRxx,    MOD(MT), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TLxx,    REG(RS), REG(RS)))

#define cmpzx_mr(MS, DP, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TLxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
        EMITW(0x7C000378 | MSM(TRxx,    REG(RT), REG(RT)))

/***************** pointer-sized instructions for 64-bit mode *****************/

/* jmp
 * set-flags: no
 * maximum byte-address-range for un/conditional jumps is signed 18/16-bit
 * based on minimum natively-encoded offset across supported targets (u/c)
 * MIPS:18-bit, Power:26-bit, AArch32:26-bit, AArch64:28-bit, x86:32-bit /
 * MIPS:18-bit, Power:16-bit, AArch32:26-bit, AArch64:21-bit, x86:32-bit */

#define jmpxx_xr(RS)           /* register-targeted unconditional jump */   \
        EMITW(0x7C0003A6 | MRM(REG(RS), 0x00,    0x09)) /* ctr <- reg */    \
        EMITW(0x4C000420 | MTM(0x0C,    0x0A,    0x00)) /* beqctr cr2 */

#define jmpxx_xm(MS, DP)         /* memory-targeted unconditional jump */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    MOD(MS), VAL(DP), C1(DP), EMPTY2)   \
        EMITW(0xE8000000 | MDM(TMxx,    MOD(MS), VAL(DP), B1(DP), P1(DP)))  \
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

#endif /* RT_RTARCH_P64_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

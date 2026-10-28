/******************************************************************************/
/* Copyright (c) 2013-2026 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTARCH_ZHB_H
#define RT_RTARCH_ZHB_H

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * rtarch_zHB.h: Implementation of s390x half+byte BASE instructions.
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
 * cmd*x_** - applies [cmd] to unsigned integer args, [x] - default
 * cmd*n_** - applies [cmd] to   signed integer args, [n] - negatable
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
 * 64/32-bit subsets are both self-consistent within themselves, 32-bit results
 * cannot be used in 64-bit subset without proper sign/zero-extend bridges,
 * cmdwn/wz bridges for 32-bit subset are provided in 64-bit headers.
 * 16/8-bit subsets are both self-consistent within themselves, their results
 * cannot be used in larger subsets without proper sign/zero-extend bridges,
 * cmdhn/hz and cmdbn/bz bridges for 16/8-bit are provided in 32-bit headers.
 * The results of 8-bit subset cannot be used within 16-bit subset consistently.
 * There is no sign/zero-extend bridge from 8-bit to 16-bit, use 32-bit instead.
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
 * Setting-flags instruction naming scheme was changed twice in the past for
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
 *
 * Only the first 4 registers are available for byte BASE logic/arithmetic and
 * shifts on legacy 32-bit targets with 8 BASE registers (ARMv7, x86).
 */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

/******************************************************************************/
/********************************   EXTERNAL   ********************************/
/******************************************************************************/

#if (defined RT_Z64)

/******************************************************************************/
/**********************************   BASE   **********************************/
/******************************************************************************/

/************************* 16-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movhx_ri(RD, IS)                                                    \
        EMITW(MHM(0xA7, REG(RD),0x08,VAL(IS)))

#define movhx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        EMITW(MHM(0xA7, TMxx,   0x08,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */

#define movhx_rr(RD, RS)                                                    \
        EMITW(MGM(0x27, REG(RD),REG(RS),0x00))

#define movhx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movhx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MDM(0x70, REG(RS),MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andhx_ri(RG, IS)                                                    \
        EMITW(MHM(0xA5, REG(RG),0x07,VAL(IS)))

#define andhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andhx_rr(RG, RS)                                                    \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define andhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define andhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andhx_mr(MG, DG, RS)                                                \
        andhx_st(W(RS), W(MG), W(DG))


#define andhxZri(RG, IS)                                                    \
        EMITW(MHM(0xA5, REG(RG),0x07,VAL(IS)))

#define andhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andhxZrr(RG, RS)                                                    \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define andhxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define andhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andhxZmr(MG, DG, RS)                                                \
        andhxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annhx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, REG(RG),0x07,VAL(IS)))

#define annhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annhx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define annhx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define annhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annhx_mr(MG, DG, RS)                                                \
        annhx_st(W(RS), W(MG), W(DG))


#define annhxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, REG(RG),0x07,VAL(IS)))

#define annhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annhxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define annhxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define annhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annhxZmr(MG, DG, RS)                                                \
        annhxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrhx_ri(RG, IS)                                                    \
        EMITW(MHM(0xA5, REG(RG),0x0B,VAL(IS)))

#define orrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrhx_rr(RG, RS)                                                    \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define orrhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))

#define orrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrhx_mr(MG, DG, RS)                                                \
        orrhx_st(W(RS), W(MG), W(DG))


#define orrhxZri(RG, IS)                                                    \
        EMITW(MHM(0xA5, REG(RG),0x0B,VAL(IS)))

#define orrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrhxZrr(RG, RS)                                                    \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define orrhxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define orrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define orrhxZmr(MG, DG, RS)                                                \
        orrhxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornhx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, REG(RG),0x0B,VAL(IS)))

#define ornhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornhx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define ornhx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))

#define ornhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornhx_mr(MG, DG, RS)                                                \
        ornhx_st(W(RS), W(MG), W(DG))


#define ornhxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, REG(RG),0x0B,VAL(IS)))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define ornhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITW(MHM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornhxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define ornhxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define ornhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define ornhxZmr(MG, DG, RS)                                                \
        ornhxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorhx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorhx_rr(RG, RS)                                                    \
        EMITH(MRM(0x17, REG(RG),REG(RS),0x00))

#define xorhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, REG(RG),TMxx,   0x00))

#define xorhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorhx_mr(MG, DG, RS)                                                \
        xorhx_st(W(RS), W(MG), W(DG))


#define xorhxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define xorhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorhxZrr(RG, RS)                                                    \
        EMITH(MRM(0x17, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define xorhxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define xorhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define xorhxZmr(MG, DG, RS)                                                \
        xorhxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define nothx_rx(RG)                                                        \
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x0000FFFF))                           \
        EMITH(0x0400 | TPxx << 4)

#define nothx_mx(MG, DG)                                                    \
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x0000FFFF))                           \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMIT6(0xE30000000004 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMITH(0x0400 | TPxx << 4)

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define neghx_rx(RG)                                                        \
        EMITH(MRM(0x13, REG(RG),REG(RG),0x00))

#define neghx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x13, TMxx,   TMxx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))


#define neghxZrx(RG)                                                        \
        EMITH(MRM(0x13, REG(RG),REG(RG),0x00))

#define neghxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x13, TMxx,   TMxx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addhx_ri(RG, IS)                                                    \
        EMITW(MHM(0xA7, REG(RG),0x0A,VAL(IS)))

#define addhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA7, TMxx,   0x0A,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addhx_rr(RG, RS)                                                    \
        EMITH(MRM(0x1A, REG(RG),REG(RS),0x00))

#define addhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define addhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addhx_mr(MG, DG, RS)                                                \
        addhx_st(W(RS), W(MG), W(DG))


#define addhxZri(RG, IS)                                                    \
        EMITW(MHM(0xA7, REG(RG),0x0A,VAL(IS)))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define addhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA7, TMxx,   0x0A,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define addhxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1A, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define addhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7A, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define addhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define addhxZmr(MG, DG, RS)                                                \
        addhxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subhx_ri(RG, IS)                                                    \
        EMITW(MHM(0xA7, REG(RG),0x0A,-VAL(IS)))

#define subhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA7, TMxx,   0x0A,-VAL(IS)))                             \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subhx_rr(RG, RS)                                                    \
        EMITH(MRM(0x1B, REG(RG),REG(RS),0x00))

#define subhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define subhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subhx_mr(MG, DG, RS)                                                \
        subhx_st(W(RS), W(MG), W(DG))


#define subhxZri(RG, IS)                                                    \
        EMITW(MHM(0xA7, REG(RG),0x0A,-VAL(IS)))                             \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define subhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MHM(0xA7, TMxx,   0x0A,-VAL(IS)))                             \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define subhxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1B, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define subhxZld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7B, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define subhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define subhxZmr(MG, DG, RS)                                                \
        subhxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlhx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDF, REG(RG),TEcx,   0x00))

#define shlhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlhx_ri(RG, IS)                                                    \
        EMIT6(MTM(0xDF, REG(RG),0x00,VAL(IS)))

#define shlhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDF, REG(RG),REG(RS),0x00))

#define shlhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x95, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDF, REG(RG),TDxx,   0x00))

#define shlhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlhx_mr(MG, DG, RS)                                                \
        shlhx_st(W(RS), W(MG), W(DG))


#define shlhxZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDF, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shlhxZri(RG, IS)                                                    \
        EMIT6(MTM(0xDF, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shlhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDF, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x95, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDF, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shlhxZmr(MG, DG, RS)                                                \
        shlhxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrhx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),TEcx,   0x00))

#define shrhx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrhx_ri(RG, IS)                                                    \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),0x00,VAL(IS)))

#define shrhx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrhx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),REG(RS),0x00))

#define shrhx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x95, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDE, REG(RG),TDxx,   0x00))

#define shrhx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrhx_mr(MG, DG, RS)                                                \
        shrhx_st(W(RS), W(MG), W(DG))


#define shrhxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shrhxZri(RG, IS)                                                    \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shrhxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x95, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x95, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDE, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shrhxZmr(MG, DG, RS)                                                \
        shrhxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrhn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),TEcx,   0x00))

#define shrhn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrhn_ri(RG, IS)                                                    \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),0x00,VAL(IS)))

#define shrhn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrhn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),REG(RS),0x00))

#define shrhn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x95, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDC, REG(RG),TDxx,   0x00))

#define shrhn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrhn_mr(MG, DG, RS)                                                \
        shrhn_st(W(RS), W(MG), W(DG))


#define shrhnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shrhnZri(RG, IS)                                                    \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shrhnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x27, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x95, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDC, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA507FFFF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrhnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x70, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA507FFFF | TMxx << 20)                 /* <- set flags (Z) */

#define shrhnZmr(MG, DG, RS)                                                \
        shrhnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulhx_ri(RG, IS)                                                    \
        EMITW(MHM(0xA7, REG(RG),0x0C,VAL(IS)))

#define mulhx_rr(RG, RS)                                                    \
        EMITW(MGM(0xFD, REG(RG),REG(RS),REG(RG)))

#define mulhx_ld(RG, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7C, REG(RG),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))


#define mulhx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(MGM(0x85, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x85, TMxx,REG(RS),   0x00))                              \
        EMITW(MGM(0xFD, TEax,   TMxx,   TEax))                              \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDE, TEdx,   0x00,   16))

#define mulhx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(MGM(0x85, TEax,   TEax,   0x00))                              \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x91, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0xFD, TEax,   TMxx,   TEax))                              \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDE, TEdx,   0x00,   16))


#define mulhn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(MGM(0x07, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x07, TMxx,REG(RS),   0x00))                              \
        EMITW(MGM(0xFD, TEax,   TMxx,   TEax))                              \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDC, TEdx,   0x00,   16))

#define mulhn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(MGM(0x07, TEax,   TEax,   0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x7C, TEax,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDC, TEdx,   0x00,   16))

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


#define prehx_xx()   /* to be placed right before divhx_x* or remhx_xx */   \
        movhx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prehn_xx()   /* to be placed right before divhn_x* or remhn_xx */   \
        movhn_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrhn_ri(Redx, IC(15))


#define divhx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(MGM(0x85, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x85, TMxx,REG(RS),   0x00))                              \
        EMITW(MGM(0x97, TEdx,   TMxx,   0x00))

#define divhx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(MGM(0x85, TEax,   TEax,   0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x95, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x97, TEdx,   TMxx,   0x00))


#define divhn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(MGM(0x07, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x07, TMxx,REG(RS),   0x00))                              \
        EMITH(MRM(0x1D, TEdx,   TMxx,   0x00))

#define divhn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(MGM(0x07, TEax,   TEax,   0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1D, TEdx,   TMxx,   0x00))

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


#define remhx_xx() /* to be placed before divhx_x*, but after prehx_xx */   \
                                     /* to prepare for rem calculation */

#define remhx_xr(RS)        /* to be placed immediately after divhx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define remhx_xm(MS, DS)    /* to be placed immediately after divhx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define remhn_xx() /* to be placed before divhn_x*, but after prehn_xx */   \
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
        AR1(W(RG), op, h, Zrx)                                              \
        CMJ(cc, lb)

#define arjhx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, h, Zmx)                                       \
        CMJ(cc, lb)

#define arjhx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, h, Zri)                                       \
        CMJ(cc, lb)

#define arjhx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, h, Zmi)                                \
        CMJ(cc, lb)

#define arjhx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, h, Zrr)                                       \
        CMJ(cc, lb)

#define arjhx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, h, Zld)                                \
        CMJ(cc, lb)

#define arjhx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, h, Zst)                                \
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

#define cmjhx_ri(RS, IT, cc, lb)                                            \
        EMITW(MGM(0x07, TMxx,   REG(RS),0x00))                              \
        EMITW(MHM(0xA7, TDxx,   0x09,VAL(IT)))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjhx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MHM(0xA7, TDxx,   0x09,VAL(IT)))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjhx_rr(RS, RT, cc, lb)                                            \
        EMITW(MGM(0x07, TMxx,   REG(RS),0x00))                              \
        EMITW(MGM(0x07, TDxx,   REG(RT),0x00))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjhx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x07, TDxx,   REG(RS),0x00))                              \
        CWR(cc, %%r13,   %%r0,   lb)

#define cmjhx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x07, TDxx,   REG(RT),0x00))                              \
        CWR(cc, %%r0,   %%r13,   lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmphx_ri(RS, IT)                                                    \
        EMITW(MGM(0x07, TLxx,   REG(RS),0x00))                              \
        EMITW(MHM(0xA7, TRxx,   0x09,VAL(IT)))

#define cmphx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MHM(0xA7, TRxx,   0x09,VAL(IT)))

#define cmphx_rr(RS, RT)                                                    \
        EMITW(MGM(0x07, TRxx,   REG(RT),0x00))                              \
        EMITW(MGM(0x07, TLxx,   REG(RS),0x00))

#define cmphx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x15, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x07, TLxx,   REG(RS),0x00))

#define cmphx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x15, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x07, TRxx,   REG(RT),0x00))

/************************** 8-bit subset instructions *************************/

/* mov (D = S)
 * set-flags: no */

#define movbx_ri(RD, IS)                                                    \
        EMITW(MBM(0xA7, REG(RD),0x08,VAL(IS)))

#define movbx_mi(MD, DD, IS)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        EMITW(MBM(0xA7, TMxx,   0x08,VAL(IS)))                              \
        EMIT6(MDM(0x72, REG(RS),MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */

#define movbx_rr(RD, RS)                                                    \
        EMITW(MGM(0x26, REG(RD),REG(RS),0x00))

#define movbx_ld(RD, MS, DS)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, REG(RD),MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))

#define movbx_st(RS, MD, DD)                                                \
        AUW(SIB(MD),  EMPTY,  EMPTY,    REG(MD), VAL(DD), A1(DD), EMPTY2)   \
        EMIT6(MDM(0x72, REG(RS),MOD(MD),REG(MD), VAL(DD), B1(DD), P1(DD)))

/* and (G = G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define andbx_ri(RG, IS)                                                    \
        EMITW(MBM(0xA5, REG(RG),0x07,VAL(IS)))

#define andbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MBM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andbx_rr(RG, RS)                                                    \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define andbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define andbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andbx_mr(MG, DG, RS)                                                \
        andbx_st(W(RS), W(MG), W(DG))


#define andbxZri(RG, IS)                                                    \
        EMITW(MBM(0xA5, REG(RG),0x07,VAL(IS)))

#define andbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MBM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andbxZrr(RG, RS)                                                    \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define andbxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define andbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define andbxZmr(MG, DG, RS)                                                \
        andbxZst(W(RS), W(MG), W(DG))

/* ann (G = ~G & S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define annbx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, REG(RG),0x07,VAL(IS)))

#define annbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annbx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))

#define annbx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define annbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annbx_mr(MG, DG, RS)                                                \
        annbx_st(W(RS), W(MG), W(DG))


#define annbxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, REG(RG),0x07,VAL(IS)))

#define annbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annbxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITH(MRM(0x14, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define annbxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x14, REG(RG),TMxx,   0x00))

#define annbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITH(MRM(0x14, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define annbxZmr(MG, DG, RS)                                                \
        annbxZst(W(RS), W(MG), W(DG))

/* orr (G = G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define orrbx_ri(RG, IS)                                                    \
        EMITW(MBM(0xA5, REG(RG),0x0B,VAL(IS)))

#define orrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MBM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrbx_rr(RG, RS)                                                    \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define orrbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))

#define orrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrbx_mr(MG, DG, RS)                                                \
        orrbx_st(W(RS), W(MG), W(DG))


#define orrbxZri(RG, IS)                                                    \
        EMITW(MBM(0xA5, REG(RG),0x0B,VAL(IS)))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define orrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MBM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define orrbxZrr(RG, RS)                                                    \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define orrbxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define orrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define orrbxZmr(MG, DG, RS)                                                \
        orrbxZst(W(RS), W(MG), W(DG))

/* orn (G = ~G | S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define ornbx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, REG(RG),0x0B,VAL(IS)))

#define ornbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornbx_rr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))

#define ornbx_ld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))

#define ornbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornbx_mr(MG, DG, RS)                                                \
        ornbx_st(W(RS), W(MG), W(DG))


#define ornbxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, REG(RG),0x0B,VAL(IS)))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define ornbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITW(MBM(0xA5, TMxx,   0x0B,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define ornbxZrr(RG, RS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITH(MRM(0x16, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define ornbxZld(RG, MS, DS)                                                \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x16, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define ornbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMITH(MRM(0x16, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define ornbxZmr(MG, DG, RS)                                                \
        ornbxZst(W(RS), W(MG), W(DG))

/* xor (G = G ^ S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define xorbx_ri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))

#define xorbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorbx_rr(RG, RS)                                                    \
        EMITH(MRM(0x17, REG(RG),REG(RS),0x00))

#define xorbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, REG(RG),TMxx,   0x00))

#define xorbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorbx_mr(MG, DG, RS)                                                \
        xorbx_st(W(RS), W(MG), W(DG))


#define xorbxZri(RG, IS)                                                    \
        EMIT6(MIM(0xC0, REG(RG),0x07,VAL(IS)))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define xorbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define xorbxZrr(RG, RS)                                                    \
        EMITH(MRM(0x17, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define xorbxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define xorbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x17, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define xorbxZmr(MG, DG, RS)                                                \
        xorbxZst(W(RS), W(MG), W(DG))

/* not (G = ~G)
 * set-flags: no */

#define notbx_rx(RG)                                                        \
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC0, REG(RG),0x07,0x000000FF))                           \
        EMITH(0x0400 | TPxx << 4)

#define notbx_mx(MG, DG)                                                    \
        EMITW(0xB2220000 | TPxx << 4)                                       \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x04,0x08)) /* SP -= 8 */                      \
        EMIT6(0xE30000000024 | MSM(TMxx,   SPxx,0x00))                      \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MIM(0xC0, TMxx,   0x07,0x000000FF))                           \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(0xE30000000004 | MSM(TMxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMIT6(0xE30000000004 | MSM(TPxx,   SPxx,0x00))                      \
        EMIT6(MIM(0xC2, SPxx,0x08,0x08)) /* SP += 8 */                      \
        EMITH(0x0400 | TPxx << 4)

/* neg (G = -G)
 * set-flags: undefined (*_*), yes (*Z*) */

#define negbx_rx(RG)                                                        \
        EMITH(MRM(0x13, REG(RG),REG(RG),0x00))

#define negbx_mx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x13, TMxx,   TMxx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))


#define negbxZrx(RG)                                                        \
        EMITH(MRM(0x13, REG(RG),REG(RG),0x00))

#define negbxZmx(MG, DG)                                                    \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x13, TMxx,   TMxx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

/* add (G = G + S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define addbx_ri(RG, IS)                                                    \
        EMITW(MBM(0xA7, REG(RG),0x0A,VAL(IS)))

#define addbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MBM(0xA7, TMxx,   0x0A,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addbx_rr(RG, RS)                                                    \
        EMITH(MRM(0x1A, REG(RG),REG(RS),0x00))

#define addbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))

#define addbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define addbx_mr(MG, DG, RS)                                                \
        addbx_st(W(RS), W(MG), W(DG))


#define addbxZri(RG, IS)                                                    \
        EMITW(MBM(0xA7, REG(RG),0x0A,VAL(IS)))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define addbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MBM(0xA7, TMxx,   0x0A,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define addbxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1A, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define addbxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define addbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1A, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define addbxZmr(MG, DG, RS)                                                \
        addbxZst(W(RS), W(MG), W(DG))

/* sub (G = G - S)
 * set-flags: undefined (*_*), yes (*Z*) */

#define subbx_ri(RG, IS)                                                    \
        EMITW(MCM(0xA7, REG(RG),0x0A,-VAL(IS)))

#define subbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MCM(0xA7, TMxx,   0x0A,-VAL(IS)))                             \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subbx_rr(RG, RS)                                                    \
        EMITH(MRM(0x1B, REG(RG),REG(RS),0x00))

#define subbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))

#define subbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define subbx_mr(MG, DG, RS)                                                \
        subbx_st(W(RS), W(MG), W(DG))


#define subbxZri(RG, IS)                                                    \
        EMITW(MCM(0xA7, REG(RG),0x0A,-VAL(IS)))                             \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define subbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MCM(0xA7, TMxx,   0x0A,-VAL(IS)))                             \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define subbxZrr(RG, RS)                                                    \
        EMITH(MRM(0x1B, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define subbxZld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, REG(RG),TMxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define subbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITH(MRM(0x1B, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define subbxZmr(MG, DG, RS)                                                \
        subbxZst(W(RS), W(MG), W(DG))

/* shl (G = G << S)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shlbx_rx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDF, REG(RG),TEcx,   0x00))

#define shlbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlbx_ri(RG, IS)                                                    \
        EMIT6(MTM(0xDF, REG(RG),0x00,VAL(IS)))

#define shlbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDF, REG(RG),REG(RS),0x00))

#define shlbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDF, REG(RG),TDxx,   0x00))

#define shlbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shlbx_mr(MG, DG, RS)                                                \
        shlbx_st(W(RS), W(MG), W(DG))


#define shlbxZrx(RG)                     /* reads Recx for shift count */   \
        EMIT6(MTM(0xDF, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shlbxZri(RG, IS)                                                    \
        EMIT6(MTM(0xDF, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shlbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMIT6(MTM(0xDF, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDF, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shlbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDF, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shlbxZmr(MG, DG, RS)                                                \
        shlbxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), unsigned (logical)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrbx_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),TEcx,   0x00))

#define shrbx_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrbx_ri(RG, IS)                                                    \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),0x00,VAL(IS)))

#define shrbx_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrbx_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),REG(RS),0x00))

#define shrbx_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDE, REG(RG),TDxx,   0x00))

#define shrbx_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrbx_mr(MG, DG, RS)                                                \
        shrbx_st(W(RS), W(MG), W(DG))


#define shrbxZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbxZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shrbxZri(RG, IS)                                                    \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbxZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shrbxZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDE, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbxZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x94, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDE, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbxZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDE, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shrbxZmr(MG, DG, RS)                                                \
        shrbxZst(W(RS), W(MG), W(DG))

/* shr (G = G >> S), signed (arithmetic)
 * set-flags: undefined (*_*), yes (*Z*)
 * for maximum compatibility: shift count must be modulo elem-size */

#define shrbn_rx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),TEcx,   0x00))

#define shrbn_mx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrbn_ri(RG, IS)                                                    \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),0x00,VAL(IS)))

#define shrbn_mi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrbn_rr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),REG(RS),0x00))

#define shrbn_ld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDC, REG(RG),TDxx,   0x00))

#define shrbn_st(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))

#define shrbn_mr(MG, DG, RS)                                                \
        shrbn_st(W(RS), W(MG), W(DG))


#define shrbnZrx(RG)                     /* reads Recx for shift count */   \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),TEcx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbnZmx(MG, DG)                 /* reads Recx for shift count */   \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   TEcx,   0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shrbnZri(RG, IS)                                                    \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),0x00,VAL(IS)))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbnZmi(MG, DG, IS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   0x00,VAL(IS)))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shrbnZrr(RG, RS)       /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        EMIT6(MTM(0xDC, REG(RG),REG(RS),0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbnZld(RG, MS, DS)   /* Recx cannot be used as first operand */   \
        EMITW(MGM(0x26, REG(RG),REG(RG),0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, TDxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMIT6(MTM(0xDC, REG(RG),TDxx,   0x00))                              \
        EMITW(0xA50700FF | REG(RG) << 20)              /* <- set flags (Z) */

#define shrbnZst(RS, MG, DG)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMIT6(MTM(0xDC, TMxx,   REG(RS),0x00))                              \
        EMIT6(MDM(0x72, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(0xA50700FF | TMxx << 20)                 /* <- set flags (Z) */

#define shrbnZmr(MG, DG, RS)                                                \
        shrbnZst(W(RS), W(MG), W(DG))

/* mul (G = G * S)
 * set-flags: undefined */

#define mulbx_ri(RG, IS)                                                    \
        EMITW(MBM(0xA7, REG(RG),0x0C,VAL(IS)))

#define mulbx_rr(RG, RS)                                                    \
        EMITW(MGM(0xFD, REG(RG),REG(RS),REG(RG)))

#define mulbx_ld(RG, MS, DS)                                                \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0xFD, REG(RG),TMxx,   REG(RG)))


#define mulbx_xr(RS)     /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(MGM(0x84, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x84, TMxx,REG(RS),   0x00))                              \
        EMITW(MGM(0xFD, TEax,   TMxx,   TEax))                              \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDE, TEdx,   0x00,   8))

#define mulbx_xm(MS, DS) /* Reax is in/out, Redx is out(high)-zero-ext */   \
        EMITW(MGM(0x84, TEax,   TEax,   0x00))                              \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0xFD, TEax,   TMxx,   TEax))                              \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDE, TEdx,   0x00,   8))


#define mulbn_xr(RS)     /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(MGM(0x06, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x06, TMxx,REG(RS),   0x00))                              \
        EMITW(MGM(0xFD, TEax,   TMxx,   TEax))                              \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDC, TEdx,   0x00,   8))

#define mulbn_xm(MS, DS) /* Reax is in/out, Redx is out(high)-sign-ext */   \
        EMITW(MGM(0x06, TEax,   TEax,   0x00))                              \
        AUW(SIB(MG),  EMPTY,  EMPTY,    REG(MG), VAL(DG), A1(DG), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MG),REG(MG), VAL(DG), B1(DG), P1(DG)))  \
        EMITW(MGM(0xFD, TEax,   TMxx,   TEax))                              \
        EMITW(MGM(0x16, TEdx,   TEax,   0x00))                              \
        EMIT6(MTM(0xDC, TEdx,   0x00,   8))

/* div (G = G / S)
 * set-flags: undefined */

#define divbx_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbx_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbx_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define divbn_ri(RG, IS)       /* Reax cannot be used as first operand */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbn_rr(RG, RS)                /* RG no Reax, RS no Reax/Redx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xr(W(RS))                                                     \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)

#define divbn_ld(RG, MS, DS)            /* RG no Reax, MS no Oeax/Medx */   \
        stack_st(Reax)                                                      \
        stack_st(Redx)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(W(MS), W(DS))                                              \
        stack_ld(Redx)                                                      \
        movbx_rr(W(RG), Reax)                                               \
        stack_ld(Reax)


#define prebx_xx()   /* to be placed right before divbx_x* or rembx_xx */   \
        movbx_ri(Redx, IC(0))        /* to prepare Redx for int-divide */

#define prebn_xx()   /* to be placed right before divbn_x* or rembn_xx */   \
        movbn_rr(Redx, Reax)         /* to prepare Redx for int-divide */   \
        shrbn_ri(Redx, IC(7))


#define divbx_xr(RS)     /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(MGM(0x84, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x84, TMxx,REG(RS),   0x00))                              \
        EMITW(MGM(0x97, TEdx,   TMxx,   0x00))

#define divbx_xm(MS, DS) /* Reax is in/out, Redx is in(zero)/out(junk) */   \
        EMITW(MGM(0x84, TEax,   TEax,   0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x94, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x97, TEdx,   TMxx,   0x00))


#define divbn_xr(RS)     /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(MGM(0x06, TEax,   TEax,   0x00))                              \
        EMITW(MGM(0x06, TMxx,REG(RS),   0x00))                              \
        EMITH(MRM(0x1D, TEdx,   TMxx,   0x00))

#define divbn_xm(MS, DS) /* Reax is in/out, Redx is in-sign-ext-(Reax) */   \
        EMITW(MGM(0x06, TEax,   TEax,   0x00))                              \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITH(MRM(0x1D, TEdx,   TMxx,   0x00))

/* rem (G = G % S)
 * set-flags: undefined */

#define rembx_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembx_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembx_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebx_xx()                                                          \
        divbx_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define rembn_ri(RG, IS)       /* Redx cannot be used as first operand */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_mi(Mebp, inf_SCR01(0), W(IS))                                 \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(Mebp, inf_SCR01(0))                                        \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembn_rr(RG, RS)                /* RG no Redx, RS no Reax/Redx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xr(W(RS))                                                     \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)

#define rembn_ld(RG, MS, DS)            /* RG no Redx, MS no Oeax/Medx */   \
        stack_st(Redx)                                                      \
        stack_st(Reax)                                                      \
        movbx_rr(Reax, W(RG))                                               \
        prebn_xx()                                                          \
        divbn_xm(W(MS), W(DS))                                              \
        stack_ld(Reax)                                                      \
        movbx_rr(W(RG), Redx)                                               \
        stack_ld(Redx)


#define rembx_xx() /* to be placed before divbx_x*, but after prebx_xx */   \
                                     /* to prepare for rem calculation */

#define rembx_xr(RS)        /* to be placed immediately after divbx_xr */   \
                                     /* to produce remainder Redx<-rem */

#define rembx_xm(MS, DS)    /* to be placed immediately after divbx_xm */   \
                                     /* to produce remainder Redx<-rem */


#define rembn_xx() /* to be placed before divbn_x*, but after prebn_xx */   \
                                     /* to prepare for rem calculation */

#define rembn_xr(RS)        /* to be placed immediately after divbn_xr */   \
                                     /* to produce remainder Redx<-rem */

#define rembn_xm(MS, DS)    /* to be placed immediately after divbn_xm */   \
                                     /* to produce remainder Redx<-rem */

/* arj (G = G op S, if cc G then jump lb)
 * set-flags: undefined
 * refer to individual instruction descriptions
 * to stay within special register limitations */

     /* Definitions for arj's "op" and "cc" parameters
      * are provided in 32-bit rtarch_***.h files. */

#define arjbx_rx(RG, op, cc, lb)                                            \
        AR1(W(RG), op, b, Zrx)                                              \
        CMJ(cc, lb)

#define arjbx_mx(MG, DG, op, cc, lb)                                        \
        AR2(W(MG), W(DG), op, b, Zmx)                                       \
        CMJ(cc, lb)

#define arjbx_ri(RG, IS, op, cc, lb)                                        \
        AR2(W(RG), W(IS), op, b, Zri)                                       \
        CMJ(cc, lb)

#define arjbx_mi(MG, DG, IS, op, cc, lb)                                    \
        AR3(W(MG), W(DG), W(IS), op, b, Zmi)                                \
        CMJ(cc, lb)

#define arjbx_rr(RG, RS, op, cc, lb)                                        \
        AR2(W(RG), W(RS), op, b, Zrr)                                       \
        CMJ(cc, lb)

#define arjbx_ld(RG, MS, DS, op, cc, lb)                                    \
        AR3(W(RG), W(MS), W(DS), op, b, Zld)                                \
        CMJ(cc, lb)

#define arjbx_st(RS, MG, DG, op, cc, lb)                                    \
        AR3(W(RS), W(MG), W(DG), op, b, Zst)                                \
        CMJ(cc, lb)

#define arjbx_mr(MG, DG, RS, op, cc, lb)                                    \
        arjbx_st(W(RS), W(MG), W(DG), op, cc, lb)

/* cmj (flags = S ? T, if cc flags then jump lb)
 * set-flags: undefined */

     /* Definitions for cmj's "cc" parameter
      * are provided in 32-bit rtarch_***.h files. */

#define cmjbx_rz(RS, cc, lb)                                                \
        cmjbx_ri(W(RS), IC(0), cc, lb)

#define cmjbx_mz(MS, DS, cc, lb)                                            \
        cmjbx_mi(W(MS), W(DS), IC(0), cc, lb)

#define cmjbx_ri(RS, IT, cc, lb)                                            \
        EMITW(MGM(0x26, TMxx,   REG(RS),0x00))                              \
        EMITW(MCM(0xA7, TDxx,   0x08,VAL(IT)))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjbx_mi(MS, DS, IT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MCM(0xA7, TDxx,   0x08,VAL(IT)))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjbx_rr(RS, RT, cc, lb)                                            \
        EMITW(MGM(0x26, TMxx,   REG(RS),0x00))                              \
        EMITW(MGM(0x26, TDxx,   REG(RT),0x00))                              \
        CWR(cc, %%r0,   %%r13,   lb)

#define cmjbx_rm(RS, MT, DT, cc, lb)                                        \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x26, TDxx,   REG(RS),0x00))                              \
        CWR(cc, %%r13,   %%r0,   lb)

#define cmjbx_mr(MS, DS, RT, cc, lb)                                        \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x76, TMxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x26, TDxx,   REG(RT),0x00))                              \
        CWR(cc, %%r0,   %%r13,   lb)

/* cmp (flags = S ? T)
 * set-flags: yes */

#define cmpbx_ri(RS, IT)                                                    \
        EMITW(MGM(0x06, TLxx,   REG(RS),0x00))                              \
        EMITW(MCM(0xA7, TRxx,   0x09,VAL(IT)))

#define cmpbx_mi(MS, DS, IT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MCM(0xA7, TRxx,   0x09,VAL(IT)))

#define cmpbx_rr(RS, RT)                                                    \
        EMITW(MGM(0x06, TRxx,   REG(RT),0x00))                              \
        EMITW(MGM(0x06, TLxx,   REG(RS),0x00))

#define cmpbx_rm(RS, MT, DT)                                                \
        AUW(SIB(MT),  EMPTY,  EMPTY,    REG(MT), VAL(DT), A1(DT), EMPTY2)   \
        EMIT6(MDM(0x77, TRxx,   MOD(MT),REG(MT), VAL(DT), B1(DT), P1(DT)))  \
        EMITW(MGM(0x06, TLxx,   REG(RS),0x00))

#define cmpbx_mr(MS, DS, RT)                                                \
        AUW(SIB(MS),  EMPTY,  EMPTY,    REG(MS), VAL(DS), A1(DS), EMPTY2)   \
        EMIT6(MDM(0x77, TLxx,   MOD(MS),REG(MS), VAL(DS), B1(DS), P1(DS)))  \
        EMITW(MGM(0x06, TRxx,   REG(RT),0x00))

#endif /* (defined RT_Z64) */

/******************************************************************************/
/********************************   INTERNAL   ********************************/
/******************************************************************************/

#endif /* RT_RTARCH_ZHB_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
